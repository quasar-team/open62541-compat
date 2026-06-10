/* © Copyright Paris Moschovakos, CERN, 2026.  All rights not expressly granted are reserved.
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

#ifndef OPEN62541_COMPAT_INCLUDE_UASEMAPHORE_H_
#define OPEN62541_COMPAT_INCLUDE_UASEMAPHORE_H_

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <statuscode.h>

class UaSemaphore
{
public:
    UaSemaphore( OpcUa_UInt32 initialValue, OpcUa_UInt32 maxRange ):
        m_count(initialValue),
        m_maxRange(maxRange)
    {}

    OpcUa_StatusCode wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this]{ return m_count > 0; });
        --m_count;
        return OpcUa_Good;
    }

    OpcUa_StatusCode timedWait( OpcUa_UInt32 msecTimeout )
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_condition.wait_for(lock, std::chrono::milliseconds(msecTimeout), [this]{ return m_count > 0; }))
            return OpcUa_BadTimeout;
        --m_count;
        return OpcUa_Good;
    }

    OpcUa_StatusCode post( OpcUa_UInt32 releaseCount )
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_count + releaseCount > m_maxRange)
            return OpcUa_BadInvalidArgument;
        m_count += releaseCount;
        m_condition.notify_all();
        return OpcUa_Good;
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_condition;
    OpcUa_UInt32 m_count;
    OpcUa_UInt32 m_maxRange;
};

#endif /* OPEN62541_COMPAT_INCLUDE_UASEMAPHORE_H_ */
