/* © Copyright Paris Moschovakos, CERN, 2026.  All rights not expressly granted are reserved.
 * async_operations.cpp
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

#include <chrono>

#include <async_operations.h>
#include <uadatetime.h>
#include <LogIt.h>

void AsyncReadBlock::fillTarget(bool policeTimestamps)
{
    if (!m_result)
        return;
    UA_DataValue_clear(m_target);
    UA_DataValue_copy(m_result->impl(), m_target);
    if (!policeTimestamps)
        return;
    if (!m_placeholderHasSourceTimestamp)
    {
        m_target->hasSourceTimestamp = false;
        m_target->sourceTimestamp = 0;
    }
    else if (!m_target->hasSourceTimestamp)
    {
        m_target->hasSourceTimestamp = true;
        m_target->sourceTimestamp = m_placeholderSourceTimestamp;
    }
    if (!m_placeholderHasServerTimestamp)
    {
        m_target->hasServerTimestamp = false;
        m_target->serverTimestamp = 0;
    }
    else if (!m_target->hasServerTimestamp)
    {
        m_target->hasServerTimestamp = true;
        m_target->serverTimestamp = m_placeholderServerTimestamp;
    }
}

void AsyncReadBlock::finishInline()
{
    fillTarget(false);
}

void AsyncReadBlock::finishDeferred(UA_Server* server)
{
    fillTarget(true);
    UA_StatusCode s = UA_Server_setAsyncReadResult(server, m_target);
    if (s != UA_STATUSCODE_GOOD)
        LOG(Log::WRN) << "UA_Server_setAsyncReadResult returned: " << UaStatus(s).toString().toUtf8();
}

void AsyncWriteBlock::finishDeferred(UA_Server* server)
{
    UA_StatusCode s = UA_Server_setAsyncWriteResult(server, m_target, (UA_StatusCode)m_result.statusCode());
    if (s != UA_STATUSCODE_GOOD)
        LOG(Log::WRN) << "UA_Server_setAsyncWriteResult returned: " << UaStatus(s).toString().toUtf8();
}

UaStatus AsyncMethodBlock::finishCall(
    OpcUa_UInt32,
    UaStatusCodeArray&,
    UaDiagnosticInfos&,
    UaVariantArray& outputArguments,
    UaStatus& statusCode)
{
    m_outputs = outputArguments;
    m_result = statusCode;
    if (phase().exchange(Finished) == Deferred)
    {
        std::shared_ptr<AsyncMethodBlock> self = m_self.lock();
        if (self)
            operations()->push(self);
    }
    return OpcUa_Good;
}

void AsyncMethodBlock::copyOutputs()
{
    size_t n = m_outputs.size() < m_outputSize ? m_outputs.size() : m_outputSize;
    for (size_t i = 0; i < n; ++i)
        UA_Variant_copy(m_outputs[i].impl(), &m_outputArray[i]);
}

void AsyncMethodBlock::finishInline()
{
    copyOutputs();
}

void AsyncMethodBlock::finishDeferred(UA_Server* server)
{
    copyOutputs();
    UA_StatusCode s = UA_Server_setAsyncCallMethodResult(server, m_outputArray, (UA_StatusCode)m_result.statusCode());
    if (s != UA_STATUSCODE_GOOD)
        LOG(Log::WRN) << "UA_Server_setAsyncCallMethodResult returned: " << UaStatus(s).toString().toUtf8();
}

AsyncOperations::AsyncOperations(UA_Server* server):
    m_server(server),
    m_serving(false),
    m_closed(false),
    m_inflight(0)
{}

bool AsyncOperations::deferralActive()
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_serving && !m_closed;
}

bool AsyncOperations::isClosed()
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_closed;
}

void AsyncOperations::setServing()
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_serving = true;
}

void AsyncOperations::establishDeferral(std::shared_ptr<AsyncOperationBlock> block)
{
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        ++m_inflight;
    }
    m_pending[block->key()] = block;
}

void AsyncOperations::push(std::shared_ptr<AsyncOperationBlock> block)
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    if (!m_closed)
        m_queue.push_back(block);
    --m_inflight;
    m_queueCv.notify_all();
}

void AsyncOperations::drain(UA_Server* server)
{
    std::vector<std::shared_ptr<AsyncOperationBlock> > batch;
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        batch.swap(m_queue);
    }
    for (auto& block : batch)
    {
        auto it = m_pending.find(block->key());
        if (it == m_pending.end() || it->second != block)
        {
            LOG(Log::INF) << "Dropping completion of a cancelled asynchronous operation";
            continue;
        }
        block->finishDeferred(server);
        m_pending.erase(it);
    }
}

void AsyncOperations::cancel(const void* key)
{
    if (m_pending.erase(key) > 0)
        LOG(Log::INF) << "Asynchronous operation cancelled by the stack (timeout, session close or cancel request)";
}

void AsyncOperations::shutdown()
{
    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_closed = true;
    if (!m_queueCv.wait_for(lock, std::chrono::seconds(60), [this]{ return m_inflight == 0; }))
        LOG(Log::ERR) << "Timed out waiting for in-flight asynchronous operations to resolve; continuing shutdown";
}

void AsyncOperations::clearRegistry()
{
    m_pending.clear();
}

void AsyncOperations::serverCancelCallback(UA_Server* server, const void* out)
{
    AsyncOperations* self = fromServer(server);
    if (self)
        self->cancel(out);
}

void AsyncOperations::drainCallback(UA_Server* server, void* data)
{
    static_cast<AsyncOperations*>(data)->drain(server);
}

AsyncOperations* AsyncOperations::fromServer(UA_Server* server)
{
    return static_cast<AsyncOperations*>(UA_Server_getConfig(server)->context);
}

AsyncReadHandle::~AsyncReadHandle()
{
    if (m_block)
        complete(UaDataValue(UaVariant(), OpcUa_BadInternalError, UaDateTime::now(), UaDateTime::now()));
}

void AsyncReadHandle::complete(const UaDataValue& result)
{
    if (!m_block)
        return;
    std::shared_ptr<AsyncReadBlock> block;
    block.swap(m_block);
    block->setResult(result);
    if (block->phase().exchange(AsyncOperationBlock::Finished) == AsyncOperationBlock::Deferred)
        block->operations()->push(block);
}

AsyncWriteHandle::~AsyncWriteHandle()
{
    if (m_block)
        complete(OpcUa_BadInternalError);
}

void AsyncWriteHandle::complete(UaStatus status)
{
    if (!m_block)
        return;
    std::shared_ptr<AsyncWriteBlock> block;
    block.swap(m_block);
    block->setResult(status);
    if (block->phase().exchange(AsyncOperationBlock::Finished) == AsyncOperationBlock::Deferred)
        block->operations()->push(block);
}
