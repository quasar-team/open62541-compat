# Async parity for the open62541 backend — design

Status: revision 2, post adversarial review (§8). Target branches: `async-parity-open62541-15` in `open62541-compat` and `quasar`.

## 1. Problem

Under the UA-SDK backend, quasar device logic never executes on the stack's network threads: method bodies declared `executionSynchronicity=asynchronous` run on `Quasar::ThreadPool` jobs (`designToClassBody.jinja:866`), and source-variable reads/writes run as pool jobs driven by `ASSourceVariableIoManager` (IOManager begin/finish transactions). The server stays responsive during slow device I/O.

Under the open62541 backend, all three glue callbacks (`unifiedRead`, `unifiedWrite`, `unifiedCall` — `src/nodemanagerbase.cpp`) execute on the single `UA_Server_run_iterate` thread (`src/uaserver.cpp:85`) and must return the result before returning. Consequently OPCUA-3385 implemented source variables as synchronous delegate calls (`ASSourceVariable::value()` invokes `ReadFn` inline) and OPCUA-3387 had to run async method bodies inline ("the thread-pool dispatch stays UASDK-only"). One slow device read or method body stalls every session, subscription, and publish of the endpoint.

**Parity definition** (what "match UASDK usage" means, measurably):

- P1. quasar `executionSynchronicity=asynchronous` methods dispatch to `SourceVariables_getThreadPool()` under both backends; the server answers concurrent requests while the method body runs.
- P2. Source-variable reads and writes execute as pool jobs under both backends; concurrent requests are served while device I/O is in flight.
- P3. Address space, value semantics, status codes, and timestamps are unchanged with respect to the current open62541 backend and equivalent to UASDK behavior (existing quasar CI nodeset comparisons keep passing).
- P4. Sync-flavor methods and cache variables behave exactly as today (zero behavior change where parity is already adequate).

## 2. Enabler: open62541 1.5

The bundled open62541 v1.2.2 cannot defer responses: its async-operation enum contains only `UA_ASYNCOPERATIONTYPE_CALL` with `READ`/`WRITE` as commented-out placeholders (`extern/open62541/include/open62541.h:26543`), and the amalgamation was produced without async support (`prepare_open62541.sh`). open62541 1.5 supports deferring read, write, and call: the operation callback returns `UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY` and the result is posted later — explicitly from a worker thread — via `UA_Server_setAsyncReadResult` / `UA_Server_setAsyncWriteResult` / the call-result equivalent, with `asyncOperationTimeout` and `maxAsyncOperationQueueSize` config and operation-cancellation notification. Verified against the v1.5.4 sources (full API map in the engineering log):

- The 1.2-era queue model (`UA_Server_setMethodNodeAsync` + worker pull + `UA_Server_setAsyncOperationResult`) is removed. Async is per-invocation: the operation callback returns `UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY`; the pending operation is created by the stack *upon that return*.
- Completion (all `UA_THREADSAFE`): `UA_Server_setAsyncCallMethodResult(server, UA_Variant *output, UA_StatusCode)` keyed by the method callback's output array pointer; `UA_Server_setAsyncReadResult(server, UA_DataValue *result)` keyed by the read callback's value pointer; `UA_Server_setAsyncWriteResult(server, const UA_DataValue *value, UA_StatusCode)`. All return `BADNOTFOUND` when no pending operation matches (cancelled / timed out / not yet created).
- Cancellation: `config->asyncOperationCancelCallback(server, const void *out)` fires with the same identifying pointer; the output memory is invalid afterwards. Config: `asyncOperationTimeout` (ms), `maxAsyncOperationQueueSize`.
- No build flag gates async (`UA_ENABLE_ASYNCOPERATIONS` no longer exists); thread-safe external calls require `UA_MULTITHREADING >= 100`, which is the 1.5 default on POSIX/Windows and is baked into the amalgamation at generation time.
- Two consequences shape §3: (a) completing *before* returning from the callback must bypass the async API entirely (the operation does not exist yet — `BADNOTFOUND`); (b) completing *after* must not write into the stack-owned result memory from a worker thread, because cancellation (EventLoop thread) frees it — worker-side writes race the free. Deferred completions therefore resolve on the EventLoop thread; the transport carrying them there is specified in §3.1 (revised after adversarial review — an earlier `UA_Server_addTimedCallback` design was rejected for deadlock and wake-latency defects, see §8).

Decision: bump the bundled stack to the latest v1.5.x and build the async parity on the stack's own deferral mechanism. Rejected alternatives:

- R1. Worker-thread + blocking handshake on 1.2.2 (pool runs the body, iterate thread waits on a future): moves device code off the iterate thread but keeps the endpoint blocked — fails P1/P2.
- R2. Port to 1.4.x first: identical porting cost (the breaking step is 1.3→1.4), gains async methods only — fails P2; lands on an aging series.
- R3. compat-private worker pool that drains queued operations: introduces a second thread-pool policy that quasar already owns; duplicate configuration, no benefit — rejected for minimality.

## 3. Architecture

Dependency direction is preserved: compat exposes UASDK-shaped deferral seams; quasar owns all threading policy (which pool, which jobs, which mutexes).

### 3.1 Completion transport (revised after adversarial review)

Verified facts that shape the transport: the server's `serviceMutex` is held while user callbacks run, and `UA_Server_addTimedCallback` takes it — so a worker holding a quasar device mutex must never call any server API (lock cycle through inline-under-mutex device code on the iterate thread). Timed callbacks also do not wake a sleeping EventLoop (only `el->cancel` writes the self-pipe; the stack wakes itself explicitly in its own async completion path). Server-internal locks are reentrant: server APIs are callable from inside server callbacks.

Therefore workers never touch the stack. The only worker-side action is pushing the completed operation block onto a compat-owned completion queue (plain mutex + vector; no other lock is ever taken while holding it). The EventLoop drains the queue at two points: in `UaServer::runThread` between `UA_Server_run_iterate` calls, and in a compat-registered repeated server callback (20 ms period) so that completions are bounded-latency even when the EventLoop sleeps in select() — the drain executes `finishDeferred` (result-memory writes + `UA_Server_setAsync*Result`) on the EventLoop thread, where it is serialized with cancellation by construction; calling the completion functions from inside the repeated callback is legal because the server locks are reentrant. Residual deviation, accepted and measured: deferred completion latency ≤ one drain period (≤20 ms) on an otherwise idle server; UASDK completes directly from the pool thread.

### 3.2 Operation blocks, handshake, ownership

One primitive serves methods, reads, and writes: a heap `AsyncOperationBlock` (`shared_ptr`-owned everywhere) carrying the stack's identifying pointer (the operation key), the payload slot, and a single-word atomic phase. The payload always lives in the block — never in the registry — so a worker completing late writes only memory it co-owns.

Two-phase handshake (release/acquire): the completing side stores the payload then `phase.exchange(Finished)`; if the prior value was `Deferred` it pushes the block to the completion queue. The glue side (`unifiedRead`/`unifiedWrite`/`unifiedCall`), after invoking `beginRead`/`beginWrite`/`beginCall`, does `phase.exchange(Deferred)`; if the prior value was `Finished` it consumes inline — for reads it writes the stack's `UA_DataValue` and returns the operation status while still inside the callback (no async operation exists; the async API is never touched); otherwise it registers the block in the pending registry and returns `GOODCOMPLETESASYNCHRONOUSLY`. Exactly one side completes on every path, including the race where the pool job finishes before the glue regains control.

Pending registry: per-server map key → `shared_ptr<block>`, mutated only on the EventLoop thread (creation in the glue, retirement in `config->asyncOperationCancelCallback`, consumption in the drain). The drain completes an entry only when both the key is present and the registered block is the same object (`it->second == block`) — pointer-identity of the compat-owned block, which the allocator cannot recycle while referenced, closes the ABA hole left by stack-side key-pointer reuse after cancellation (cancellation is routine: `asyncOperationTimeout` — stack default 120 s, session close, OPC UA CancelRequest, monitored-item deletion). A cancelled operation thus resolves to: registry entry erased; the worker still completes into the queue; the drain finds no matching identity and drops; the block frees via refcount. `BADNOTFOUND` from completion calls is impossible-by-construction yet still tolerated.

### 3.3 Methods

`addMethodNodeAndReference` is unchanged (deferral is per-call via the return code). `unifiedCall` allocates an `AsyncMethodBlock` (an `AsyncOperationBlock` that is also the `MethodManagerCallback`) carrying the stack's `UA_Variant* output` array and arity, and invokes `receiver->beginCall` exactly as today. Generated sync-flavor methods call `finishCall` inline → consumed inline, observable behavior unchanged (P4). Async-flavor methods dispatch to `SourceVariables_getThreadPool()` and `finishCall` later runs on the pool thread → handshake → queue → drain → outputs copied (bounded by `min(stored, arity)` — the previous unconditional copy was a latent `std::out_of_range` crash for throwing methods with return values, fixed here) → `UA_Server_setAsyncCallMethodResult`.

Failure paths: argument-conversion errors return bad before any dispatch (phase still Initial) → synchronous error, block freed by refcount. Generated exception handlers route through `finishCall` with bad status (empty outputs — handled by the bounded copy). The generated dispatch must check `addJob`'s status (today it is discarded — under a full pool the client would hang until `asyncOperationTimeout`): on failure the template calls `finishCall` with the pool status, which the handshake turns into a synchronous error return; this also fixes the same latent hang in the UASDK path.

### 3.4 Source variables

UASDK reference semantics as before (pool for `asynchronous`, inline-under-mutex for `synchronous`, six mutex modes, try/catch → `BadInternalError`, status passthrough, `BadOutOfService` on pool-down). compat seam on `OpcUa::BaseDataVariableType`, default-off so cache variables keep the exact current hot path (zero extra copies — deliberate, sampling reads are hot):

```
virtual OpcUa_Boolean handlesIo() const;                 // default OpcUa_False
virtual void beginRead (AsyncReadHandle h);
virtual void beginWrite (const UaDataValue& v, AsyncWriteHandle h);
```

Handles are move-only wrappers over the block; `complete()` runs the handshake; a dropped-without-complete handle completes `OpcUa_BadInternalError` (RAII). quasar `ASSourceVariable` (open62541 branch) dispatches per the design attributes: async-declared → `addJob(wrapper, description, mutex)` with `addJob` failure or null pool completing `OpcUa_BadOutOfService`; sync-declared → inline execute, taking `std::unique_lock` only when a mutex is configured (the schema default `no` must not dereference a null mutex). Mutex resolution happens at request time through a generated resolver replicating the UASDK IoJob constructor mapping, including: null device link → `OpcUa_BadInternalError` (resolution runs before the ReadFn's own null-check), `handpicked` resolving null → `OpcUa_BadInternalError`. The wrapper try/catches the device call and builds `UaDataValue(value, status, sourceTime, now)` with status passthrough (supersedes OPCUA-3385's cached-value fallback — deliberate UASDK-parity deviation, recorded in the parity report).

Two stack-truth obligations on the read path:

- **Timestamp order**: compat's `UaDataValue` constructor takes `(value, status, serverTime, sourceTime)` — the reverse of the genuine UASDK `(value, status, sourceTime, serverTime)`. Code written against UASDK conventions (including the existing o6 `ASSourceVariable`) silently swaps timestamps; the sync path masked it because the stack re-stamps, the deferred path sends them verbatim. The constructor parameter order is aligned to UASDK as part of this work (a pre-existing parity bug in its own right).
- **TimestampsToReturn policing**: the stack applies the client's `TimestampsToReturn` to the placeholder `UA_DataValue` at defer time and never re-polices after `UA_Server_setAsyncReadResult`. `finishDeferred` therefore merges rather than overwrites: it preserves the placeholder's `has{Source,Server}Timestamp` flags (and their defer-time values when the result does not supply one), so ttr=Neither/Source/Server behave exactly as the sync path and UASDK.

Monitored items (verified, no longer open): sampling always goes through the same `UA_CallbackValueSource` read callback via the stack's internal `read_async`, so deferral on the sampling path comes for free; each sample on an async-declared source variable is a pool job (pool sizing now gates publish health — parity-equivalent with UASDK pool usage); outstanding sampling reads are capped at `UA_MONITOREDITEM_ASYNC_MAX = 8` per item (slower-than-interval devices degrade to overflow notifications, vs UASDK's `BadOutOfService` at `maxJobs`); monitored-item deletion cancels outstanding reads — the cancel path is hot, not exceptional.

### 3.5 Lifetime and shutdown (against quasar's real teardown)

quasar's actual order (verified in `BaseQuasarServer::serverRun`): running flag cleared → `SourceVariables_destroySourceVariablesThreadPool()` (joins running jobs, drops queued ones) → device unlink → `UaServer::stop()`. So by the time `stop()` runs under quasar, workers are already gone; the compat machinery must additionally survive the standalone/gtest case where compat owns the only threads.

- Queue gate: `closed` flag + `inflight` count inside the queue mutex. `inflight` increments when the glue establishes a deferral (EventLoop thread; the queue mutex protects nothing that is ever held across stack or device calls, so no inversion) and decrements on every worker resolution — push, or drop-when-closed. `stop()` waits with a 60 s timeout and a loud ERR on expiry (a wedged device call is diagnosable, not a silent hang).
- Once `closed`, the glue refuses new deferrals: `unifiedRead`/`unifiedWrite`/`unifiedCall` return `BADOUTOFSERVICE` (matters because `UA_Server_run_shutdown` runs full iterations that can still deliver client requests).
- `UA_Server_run_shutdown` stays in `runThread` (today's structure). Pending repeated-callback drains may fire during shutdown iterations against the still-alive registry — legitimate completions.
- `stop()` order: join iterate thread → close gate, wait `inflight == 0` → final queue drain (server still alive; late completions either match the registry and complete or drop) → registry clear → `UA_Server_delete` (its internal `UA_AsyncManager_clear` fires cancel callbacks at delete time — the cancel callback only touches the registry, which is empty, via `config->context`, which is freed only after) → delete the `AsyncOperations` instance → `delete m_nodeManager` last, so no compat code path can touch nodes after their deletion. Registry, gate, and queue are per-`UaServer` members reached via `config->context` (set in `start()` after `UA_ServerConfig_setMinimal`, before `UA_Server_run_startup`) — no static state, multiple in-process servers (the gtest plan) stay isolated.

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

## 8. Adversarial review record

Four independent reviewers (threading/races, lifetime/shutdown, UASDK fidelity, minimality/API) ran against revision 1 of this document plus the compat/quasar sources and the open62541 v1.5.4 sources. All four returned *revise*. Disposition:

- Trampoline-via-`UA_Server_addTimedCallback` deadlock (serviceMutex held during callbacks; worker holds device mutex) and missing EventLoop wake (≤500 ms stalls): **replaced** by the worker-push queue + EventLoop drain + 20 ms repeated callback (§3.1).
- Registry ABA under stack key-pointer reuse after cancel: **fixed** by block-identity comparison in the drain (§3.2).
- Payload-slot ownership contradictions (worker writes vs EventLoop-only registry vs cancel-frees): **resolved** — payload is block-owned, registry is a liveness map, cancel only retires (§3.2).
- inflight only decremented on the closed branch (wedged stop()); gate touched from EventLoop contrary to its own spec: **fixed** — always-decrement, queue-mutex accounting with stated lock order (§3.5).
- stop() specified against the wrong quasar teardown; `run_shutdown` relocation contradiction; `UA_AsyncManager_clear` firing cancels at delete; nodeManager deletion ordering; multi-instance static-state hazard: **all addressed** in §3.5.
- `UaDataValue` constructor timestamp order reversed vs UASDK (pre-existing, exposed by deferred path): **fixed in this series** (§3.4).
- Deferred completions bypassing TimestampsToReturn policing: **fixed** — flag-preserving merge (§3.4).
- Monitored-item sampling always defers; `UA_MONITOREDITEM_ASYNC_MAX=8`; cancel is hot-path: **folded in** (§3.4); subscription case added to the test plan.
- Sync-declared + `addressSpaceReadUseMutex="no"` null-mutex dereference; null device link during mutex resolution; `handpicked` status pinned to `BadInternalError`: **specified** (§3.4). (The UASDK template's own `'no '` trailing-space defect at `designToSourceVariablesBody.jinja:456` is noted for a separate upstream fix.)
- Generated async dispatch discarding `addJob` status (120 s client hang under full pool; leak): **template now checks and finishes with the pool status** (§3.3, §4).
- Method output copy unbounded (`std::out_of_range` through the C stack for throwing methods with return values — also latent in today's `unifiedCall`): **bounded copy** (§3.3).
- Seam-shrink suggestion (drop `handlesIo()`): **declined** — keeping the cache-variable hot path copy-free; rationale recorded in §3.4.
- Direct worker completion for writes (no pointer dereference in `setAsyncWriteResult`): **declined** — one uniform transport outweighs the micro-optimization.
