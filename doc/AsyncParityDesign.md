# Async parity for the open62541 backend — design

Status: draft for adversarial review. Target branches: `async-parity-open62541-15` in `open62541-compat` and `quasar`.

## 1. Problem

Under the UA-SDK backend, quasar device logic never executes on the stack's network threads: method bodies declared `executionSynchronicity=asynchronous` run on `Quasar::ThreadPool` jobs (`designToClassBody.jinja:866`), and source-variable reads/writes run as pool jobs driven by `ASSourceVariableIoManager` (IOManager begin/finish transactions). The server stays responsive during slow device I/O.

Under the open62541 backend, all three glue callbacks (`unifiedRead`, `unifiedWrite`, `unifiedCall` — `src/nodemanagerbase.cpp`) execute on the single `UA_Server_run_iterate` thread (`src/uaserver.cpp:85`) and must return the result before returning. Consequently OPCUA-3385 implemented source variables as synchronous delegate calls (`ASSourceVariable::value()` invokes `ReadFn` inline) and OPCUA-3387 had to run async method bodies inline ("the thread-pool dispatch stays UASDK-only"). One slow device read or method body stalls every session, subscription, and publish of the endpoint.

**Parity definition** (what "match UASDK usage" means, measurably):

- P1. quasar `executionSynchronicity=asynchronous` methods dispatch to `SourceVariables_getThreadPool()` under both backends; the server answers concurrent requests while the method body runs.
- P2. Source-variable reads and writes execute as pool jobs under both backends; concurrent requests are served while device I/O is in flight.
- P3. Address space, value semantics, status codes, and timestamps are unchanged with respect to the current open62541 backend and equivalent to UASDK behavior (existing quasar CI nodeset comparisons keep passing).
- P4. Sync-flavor methods and cache variables behave exactly as today (zero behavior change where parity is already adequate).

## 2. Enabler: open62541 1.5

The bundled open62541 v1.2.2 cannot defer responses: its async-operation enum contains only `UA_ASYNCOPERATIONTYPE_CALL` with `READ`/`WRITE` as commented-out placeholders (`extern/open62541/include/open62541.h:26543`), and the amalgamation was produced without async support (`prepare_open62541.sh`). open62541 1.5 supports deferring read, write, and call: the operation callback returns `UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY` and the result is posted later — explicitly from a worker thread — via `UA_Server_setAsyncReadResult` / `UA_Server_setAsyncWriteResult` / the call-result equivalent, with `asyncOperationTimeout` and `maxAsyncOperationQueueSize` config and operation-cancellation notification. Exact signatures, build flags, and thread-safety guarantees are being extracted from the v1.5.4 sources; all names below marked ⟨1.5-API⟩ are to be bound to the verified signatures before implementation.

Verified against the v1.5.4 sources (full API map in the engineering log):

- The 1.2-era queue model (`UA_Server_setMethodNodeAsync` + worker pull + `UA_Server_setAsyncOperationResult`) is removed. Async is per-invocation: the operation callback returns `UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY`; the pending operation is created by the stack *upon that return*.
- Completion (all `UA_THREADSAFE`): `UA_Server_setAsyncCallMethodResult(server, UA_Variant *output, UA_StatusCode)` keyed by the method callback's output array pointer; `UA_Server_setAsyncReadResult(server, UA_DataValue *result)` keyed by the read callback's value pointer; `UA_Server_setAsyncWriteResult(server, const UA_DataValue *value, UA_StatusCode)`. All return `BADNOTFOUND` when no pending operation matches (cancelled / timed out / not yet created).
- Cancellation: `config->asyncOperationCancelCallback(server, const void *out)` fires with the same identifying pointer; the output memory is invalid afterwards. Config: `asyncOperationTimeout` (ms), `maxAsyncOperationQueueSize`.
- No build flag gates async (`UA_ENABLE_ASYNCOPERATIONS` no longer exists); thread-safe external calls require `UA_MULTITHREADING >= 100`, which is the 1.5 default on POSIX/Windows and is baked into the amalgamation at generation time.
- Two consequences shape §3: (a) completing *before* returning from the callback must bypass the async API entirely (the operation does not exist yet — `BADNOTFOUND`); (b) completing *after* must not write into the stack-owned result memory from a worker thread, because cancellation (EventLoop thread) frees it — worker-side writes race the free, and guarding both sides with one compat mutex inverts against the server's internal lock. Deferred completions therefore hop through the EventLoop via `UA_Server_addTimedCallback` (zero-delay), where they are inherently serialized with cancellation; the stack's own server-local async API documents the same pattern ("the result-callback is executed only in the next iteration of the Eventloop").

Decision: bump the bundled stack to the latest v1.5.x and build the async parity on the stack's own deferral mechanism. Rejected alternatives:

- R1. Worker-thread + blocking handshake on 1.2.2 (pool runs the body, iterate thread waits on a future): moves device code off the iterate thread but keeps the endpoint blocked — fails P1/P2.
- R2. Port to 1.4.x first: identical porting cost (the breaking step is 1.3→1.4), gains async methods only — fails P2; lands on an aging series.
- R3. compat-private worker pool that drains queued operations: introduces a second thread-pool policy that quasar already owns; duplicate configuration, no benefit — rejected for minimality.

## 3. Architecture

Dependency direction is preserved: compat exposes UASDK-shaped deferral seams; quasar owns all threading policy (which pool, which jobs, which mutexes).

### 3.1 Methods

`unifiedCall` becomes deferral-capable; the decision to defer is made by the generated quasar code, not by compat:

1. `addMethodNodeAndReference` is unchanged (no per-node marking exists in 1.5; deferral is decided per-call by the return code).
2. `unifiedCall` allocates an `AsyncMethodCallback : MethodManagerCallback` on the heap carrying the server pointer, the stack's `UA_Variant *output` array (the operation identity), the output arity, and a two-phase atomic handshake. It invokes `receiver->beginCall(cb, …)` exactly as today.
3. Generated `call<Name>()`:
   - sync flavor: runs the body inline and calls `finishCall` before returning — `unifiedCall` detects inline completion via the handshake, copies the stored outputs into the stack's output array on the spot (legal: still inside the method callback, no async operation exists), frees the callback, and returns the final status synchronously. Identical observable behavior to today (P4).
   - async flavor: dispatches the body to `SourceVariables_getThreadPool()` and returns — `unifiedCall` detects no-completion-yet and returns `UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY`, creating the pending operation. The pool job later calls `finishCall`, which stores outputs+status into the callback object and posts a zero-delay `UA_Server_addTimedCallback` trampoline; the trampoline (EventLoop thread) consults the pending-operation registry, copies outputs into the stack's output array, calls `UA_Server_setAsyncCallMethodResult`, and frees the callback.

Handshake (single-word atomic, release/acquire): `finishCall` stores results then `phase.exchange(FINISHED)`; if the previous value was `DEFERRED` it arms the trampoline. `unifiedCall` after `beginCall` does `phase.exchange(DEFERRED)`; if the previous value was `FINISHED` it consumes inline. Exactly one party completes on every path, including the race where the pool job finishes before `beginCall` returns to `unifiedCall`.

Failure paths: `beginCall` (or argument conversion in `call<Name>`) returning bad without `finishCall` → synchronous error return, callback freed by `unifiedCall`. Generated code's exception handlers already route through `finishCall` with bad status — no change required. `setAsyncCallMethodResult` returning `BADNOTFOUND` (operation cancelled or timed out between trampoline arming and execution) → drop and free, no output writes (the registry entry was already retired by the cancel callback, so the trampoline never touches the dead output pointer).

### 3.2 Source variables

The UASDK reference semantics (`designToSourceVariablesBody.jinja`, `SourceVariables_spawnIoJob{Read,Write}`):

- `addressSpaceRead/Write="asynchronous"` → pool job (`ThreadPool::addJob`, mutex-aware scheduling via `ThreadPoolJob::associatedMutex`).
- `"synchronous"` → executed inline in the calling thread under `std::unique_lock` of the associated mutex.
- Mutex selection per design attribute: `of_this_operation` / `of_this_variable` / `of_containing_object` / `of_parent_of_containing_object` / `handpicked` / `no`, resolved through device-logic lock getters at request time.
- Device call wrapped in try/catch → `OpcUa_BadInternalError`; result is `UaDataValue(value, status, sourceTime, now)` with the device status passed through (no cached-value fallback).
- Thread pool unavailable → `OpcUa_BadOutOfService`.

compat `OpcUa::BaseDataVariableType` gains a minimal deferral seam, default-off:

```
class AsyncReadHandle  { void complete(const UaDataValue&); /* move-only, RAII: completes Bad if dropped */ };
class AsyncWriteHandle { void complete(UaStatus); };
virtual OpcUa_Boolean handlesIo() const;                 // default OpcUa_False
virtual void beginRead (AsyncReadHandle h);              // contract: called only when handlesIo()
virtual void beginWrite (const UaDataValue& v, AsyncWriteHandle h);
```

`unifiedRead`/`unifiedWrite`: if `handlesIo()`, construct the handle (carrying the server pointer and the stack's `UA_DataValue*` — the operation identity — plus the same two-phase handshake as §3.1), invoke `beginRead`/`beginWrite`, and return `GOODCOMPLETESASYNCHRONOUSLY` only if the handle was not completed inline; else the existing synchronous path runs unchanged (cache variables: zero change, P4).

`complete()` resolves through the handshake: inline (before `beginRead` returns to `unifiedRead`) → write the result directly into the stack's `UA_DataValue` and let `unifiedRead` return its status synchronously — this carries the `synchronous`-declared source variables with zero async machinery; deferred → store the payload in the handle and arm the zero-delay trampoline, which on the EventLoop thread checks the registry, writes the stack's `UA_DataValue`, and calls `UA_Server_setAsyncReadResult` (`UA_Server_setAsyncWriteResult` for writes).

quasar `ASSourceVariable` (open62541 branch) overrides the seam and replicates the UASDK policy exactly:

- ctor gains per-operation synchronicity flags and `MutexFn` (`std::function<std::mutex*()>`) resolvers generated by the jinja with the same attribute mapping as the UASDK IoJob constructors (`handpicked` resolving to null → refuse the operation, mirroring the UASDK throw path).
- `beginRead`, async-declared: `SourceVariables_getThreadPool()->addJob(wrapper, description, mutexFn())`; pool null or addJob failing → `h.complete` with `OpcUa_BadOutOfService`.
- `beginRead`, sync-declared: inline `std::unique_lock` + execute + `h.complete` (the mutex is required for correctness, not fidelity: pool-side jobs now run concurrently with the iterate thread).
- wrapper try/catches the existing `ReadFn`/`WriteFn` device calls → `OpcUa_BadInternalError`, and builds `UaDataValue(value, status, sourceTime, now)` with status passthrough. This supersedes OPCUA-3385's bad-or-empty-falls-back-to-cached-value behavior — a deliberate deviation toward UASDK parity, recorded in the parity report (the current fallback is itself a deviation that nodeset comparisons cannot see).
- the generated `ReadFn`/`WriteFn` lambdas in `designToClassBody.jinja` are untouched.

### 3.3 Lifetime, cancellation, shutdown

Two synchronization domains, with a strict lock order (gate mutex → server-internal locks; the EventLoop thread never takes the gate mutex):

- **Registry (EventLoop-thread-only, no lock).** A map keyed by the stack's identifying pointer, holding each pending operation's payload slot. All mutations happen on the iterate thread: creation in `unifiedRead`/`unifiedWrite`/`unifiedCall` when returning `GOODCOMPLETESASYNCHRONOUSLY`, retirement in `config->asyncOperationCancelCallback` (frees the payload slot; subsequent trampoline finds nothing and no-ops), consumption in trampolines (write stack result memory, call `UA_Server_setAsync*Result`, retire). Since cancellation, consumption, and creation are all serialized on one thread, compat never writes stack-owned result memory that the stack has freed; `BADNOTFOUND` is impossible-by-construction yet still handled (drop) as defense in depth.
- **Worker gate (`mutex` + `condition_variable` + `closed` + `inflight` count), touched only by worker threads and `UaServer::stop()`.** Deferred handles increment `inflight` at creation. A worker's `complete()`/RAII-drop stores the payload into its entry's slot and, holding the gate mutex, checks `closed`: open → `UA_Server_addTimedCallback` (zero-delay trampoline; safe to call from any thread; only enqueues, needs no running iterate); closed → decrement and notify without touching the server. Holding the gate mutex across the check+arm closes the check-then-act race against deletion; no inversion is possible because no EventLoop-side code takes the gate mutex.
- **`UaServer::stop()` order**: quasar has already cleared the running flag (existing contract), so the iterate loop exits; join the thread → close the gate and wait for `inflight == 0` (bounded by the longest running device call, identical to UASDK practice) → free any remaining registry payloads (no concurrent access possible now) → `UA_Server_run_shutdown` → `UA_Server_delete`. Armed-but-unexecuted trampolines are dropped by the stack at delete; their payloads were registry-owned and already freed.
- Exactly one party completes/frees on every path: the handshake winner inline, the trampoline on the EventLoop, the cancel callback for stack-cancelled operations, or `stop()`'s drain. A dropped-without-complete handle routes through the same worker-side path with `OpcUa_BadInternalError` (RAII), so a lost pool job cannot wedge a client request until `asyncOperationTimeout`.

### 3.4 What explicitly does not change

- Sync-flavor methods keep executing inline on the iterate thread (matches quasar's declared semantics; UASDK runs them on the SDK thread that delivered the call, also without pool dispatch).
- Monitored-item sampling of source variables: if ⟨1.5-API⟩ supports deferral on the sampling path it comes for free; if not, sampling takes the synchronous path as today. To be verified; either outcome is recorded in the parity report.
- No public header changes UASDK code could observe; all additions are open62541-backend-only types and virtuals with defaults.

## 4. quasar-side change specification

On `quasar` branch `async-parity-open62541-15`:

1. `AddressSpace/templates/designToClassBody.jinja`:
   - methods: remove the two `#ifndef BACKEND_OPEN62541` / `#endif` guards around the thread-pool dispatch introduced by OPCUA-3387 (async methods then dispatch identically under both backends; sync methods stay inline under both, matching UASDK's policy of pooling only the async flavor);
   - source variables (open62541 branch): pass synchronicity flags and the per-operation `MutexFn` (same jinja attribute mapping as the UASDK IoJob constructors) into the `ASSourceVariable` constructor.
2. `AddressSpace/include/ASSourceVariable.h` (open62541 branch): the §3.2 dispatch (pool for async-declared, inline-under-mutex for sync-declared, try/catch, status passthrough, `BadOutOfService` on pool-down). Pool initialization is backend-agnostic (Meta's `DSourceVariableThreadPool`, `Meta/src/DSourceVariableThreadPool.cpp:67`).
3. Only if the device-logic lock getters (`getLockVariable_*`, `getLockVariableRead_*`, …) turn out UASDK-gated in the Device templates: un-gate them (they are plain `std::mutex` accessors with no toolkit dependency).

Residual asymmetry, accepted and documented: sync-flavor *methods* under open62541 run on the iterate thread (UASDK runs them on an SDK worker), so a slow `executionSynchronicity=synchronous` method still stalls the open62541 endpoint. This matches today's behavior, matches the design language's intent (synchronous = caller-blocking), and is fixable later by pooling sync calls too if ever needed.

## 5. open62541-compat commit series (reviewable, on `async-parity-open62541-15`)

1. `prepare_open62541.sh`: TAG → v1.5.x latest patch, plus the flags async deferral requires ⟨1.5-API⟩; regenerated amalgamation committed with the established `open62541` author convention.
2. Port commit: `typeIndex`→`typeKind` (`src/uavariant.cpp:594,648`), `config->logger`→`config->logging` (+ `UA_Logger` struct shape, init order before `UA_ServerConfig_setMinimal`) in `src/uaserver.cpp` and `src/uaclient/uasession.cpp`, amalgamation C-flags/link-libs adjustments in `CMakeLists.txt`, anything else the compiler surfaces. Behavior: sync, green gtests, both build modes.
3. Async methods commit (§3.1) + gtest.
4. Async source variables commit (§3.2, §3.3) + gtest: in-process `UaServer` with a deferring test variable; assert a fast read completes while a slow deferred read is pending (the parity property itself, unit-level).
5. Docs: README version table row; this document finalized.

Code style: match surrounding code; no comments in new code.

## 6. Test and parity-verification plan

- Unit (compat): existing suite green post-port; new async tests as above.
- quasar CI cases locally in docker (`.CI/run_test_case.py`): `test_methods`, `test_async_methods`, `test_source_variables`, `test_cache_variables` with `--opcua_backend o6` against local compat; nodeset comparisons must pass (P3).
- Server matrix: CAEN (`atlas-dcs-common-software/opcuaservers/caen-opcua-server`), SCA (`ScaOpcUa`), CanOpen (`CanOpenOpcUa`), ATCA (`AtcaOpcUa`), each × {uasdk, open62541-new}; build in docker, run without hardware where the server permits, probe:
  - address-space dump diff (ns2),
  - read/write/method functional checks,
  - responsiveness metric: latency of a concurrent cache-variable read while a slow source-variable read (or async method) is in flight — expected: uasdk fast / o6-today blocked / o6-new fast.
- Each matrix cell records: build ok, run ok, probe results, parity verdict; blocked cells get an explicit cause.

## 7. Risks

- 1.5 async API maturity (youngest feature of the youngest series): mitigated by latest-patch selection, the unit gate before any quasar test, and the registry/RAII guards.
- Amalgamation under multithreading/async flags is less-trodden: verified first thing during the port; fallback is unbundled full-source vendoring (bigger diff — only if forced).
- 1.5 behavioral deltas unrelated to async (encoding, defaults) could surface in nodeset diffs: caught by P3 comparisons.
