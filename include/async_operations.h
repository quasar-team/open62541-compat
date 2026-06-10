/* © Copyright Paris Moschovakos, CERN, 2026.  All rights not expressly granted are reserved.
 * async_operations.h
 *
 *  Created on: 9 Jun, 2026
 *      Author: Paris Moschovakos <paris.moschovakos@cern.ch>
 *
 *  This file is part of Quasar.
 *
 *  Quasar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public Licence as published by
 *  the Free Software Foundation, either version 3 of the Licence.
 *
 *  Quasar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public Licence for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Quasar.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPEN62541_COMPAT_INCLUDE_ASYNC_OPERATIONS_H_
#define OPEN62541_COMPAT_INCLUDE_ASYNC_OPERATIONS_H_

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include <open62541.h>
#include <statuscode.h>
#include <uadatavalue.h>
#include <uavariant.h>
#include <arrays.h>
#include <methodhandleuanode.h>
#include <methodmanager.h>

class AsyncOperations;

class AsyncOperationBlock
{
public:
    enum Phase { Initial = 0, Finished = 1, Deferred = 2 };

    AsyncOperationBlock(AsyncOperations* operations, const void* key):
        m_operations(operations),
        m_key(key),
        m_phase(Initial)
    {}
    virtual ~AsyncOperationBlock() {}

    virtual void finishInline() = 0;
    virtual void finishDeferred(UA_Server* server) = 0;

    AsyncOperations* operations() const { return m_operations; }
    const void* key() const { return m_key; }
    std::atomic<int>& phase() { return m_phase; }

private:
    AsyncOperationBlock(const AsyncOperationBlock&);
    void operator=(const AsyncOperationBlock&);

    AsyncOperations* m_operations;
    const void*      m_key;
    std::atomic<int> m_phase;
};

class AsyncReadBlock: public AsyncOperationBlock
{
public:
    AsyncReadBlock(AsyncOperations* operations, UA_DataValue* target):
        AsyncOperationBlock(operations, target),
        m_target(target),
        m_placeholderHasSourceTimestamp(target->hasSourceTimestamp),
        m_placeholderHasServerTimestamp(target->hasServerTimestamp),
        m_placeholderSourceTimestamp(target->sourceTimestamp),
        m_placeholderServerTimestamp(target->serverTimestamp)
    {}

    void setResult(const UaDataValue& result) { m_result.reset(new UaDataValue(result)); }
    virtual void finishInline() override;
    virtual void finishDeferred(UA_Server* server) override;

private:
    void fillTarget(bool policeTimestamps);

    UA_DataValue* m_target;
    UA_Boolean    m_placeholderHasSourceTimestamp;
    UA_Boolean    m_placeholderHasServerTimestamp;
    UA_DateTime   m_placeholderSourceTimestamp;
    UA_DateTime   m_placeholderServerTimestamp;
    std::unique_ptr<UaDataValue> m_result;
};

class AsyncWriteBlock: public AsyncOperationBlock
{
public:
    AsyncWriteBlock(AsyncOperations* operations, const UA_DataValue* target):
        AsyncOperationBlock(operations, target),
        m_target(target),
        m_result(OpcUa_BadInternalError)
    {}

    void setResult(UaStatus status) { m_result = status; }
    UaStatus result() const { return m_result; }
    virtual void finishInline() override {}
    virtual void finishDeferred(UA_Server* server) override;

private:
    const UA_DataValue* m_target;
    UaStatus            m_result;
};

class AsyncMethodBlock: public AsyncOperationBlock, public MethodManagerCallback
{
public:
    AsyncMethodBlock(AsyncOperations* operations, UA_Variant* outputArray, size_t outputSize):
        AsyncOperationBlock(operations, outputArray),
        m_outputArray(outputArray),
        m_outputSize(outputSize),
        m_result(OpcUa_BadInternalError)
    {}

    virtual UaStatus finishCall(
        OpcUa_UInt32,
        UaStatusCodeArray&,
        UaDiagnosticInfos&,
        UaVariantArray& outputArguments,
        UaStatus& statusCode) override;

    void attachSelf(std::shared_ptr<AsyncMethodBlock> self) { m_self = self; }
    UaStatus result() const { return m_result; }
    virtual void finishInline() override;
    virtual void finishDeferred(UA_Server* server) override;

private:
    void copyOutputs();

    UA_Variant*    m_outputArray;
    size_t         m_outputSize;
    UaVariantArray m_outputs;
    UaStatus       m_result;
    std::weak_ptr<AsyncMethodBlock> m_self;
};

class AsyncOperations
{
public:
    explicit AsyncOperations(UA_Server* server);

    bool deferralActive();
    bool isClosed();
    void setServing();
    void establishDeferral(std::shared_ptr<AsyncOperationBlock> block);
    void push(std::shared_ptr<AsyncOperationBlock> block);
    void drain(UA_Server* server);
    void cancel(const void* key);
    void shutdown();
    void clearRegistry();

    static void serverCancelCallback(UA_Server* server, const void* out);
    static void drainCallback(UA_Server* server, void* data);
    static AsyncOperations* fromServer(UA_Server* server);

private:
    UA_Server* m_server;
    std::map<const void*, std::shared_ptr<AsyncOperationBlock> > m_pending;
    std::mutex                                          m_queueMutex;
    std::condition_variable                             m_queueCv;
    std::vector<std::shared_ptr<AsyncOperationBlock> >  m_queue;
    bool                                                m_serving;
    bool                                                m_closed;
    unsigned int                                        m_inflight;
};

class AsyncReadHandle
{
public:
    AsyncReadHandle(std::shared_ptr<AsyncReadBlock> block): m_block(block) {}
    AsyncReadHandle(AsyncReadHandle&& other): m_block(std::move(other.m_block)) {}
    AsyncReadHandle(const AsyncReadHandle&) = delete;
    AsyncReadHandle& operator=(const AsyncReadHandle&) = delete;
    ~AsyncReadHandle();

    void complete(const UaDataValue& result);

private:
    std::shared_ptr<AsyncReadBlock> m_block;
};

class AsyncWriteHandle
{
public:
    AsyncWriteHandle(std::shared_ptr<AsyncWriteBlock> block): m_block(block) {}
    AsyncWriteHandle(AsyncWriteHandle&& other): m_block(std::move(other.m_block)) {}
    AsyncWriteHandle(const AsyncWriteHandle&) = delete;
    AsyncWriteHandle& operator=(const AsyncWriteHandle&) = delete;
    ~AsyncWriteHandle();

    void complete(UaStatus status);

private:
    std::shared_ptr<AsyncWriteBlock> m_block;
};

#endif /* OPEN62541_COMPAT_INCLUDE_ASYNC_OPERATIONS_H_ */
