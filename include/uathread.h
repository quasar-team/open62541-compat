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

#ifndef OPEN62541_COMPAT_INCLUDE_UATHREAD_H_
#define OPEN62541_COMPAT_INCLUDE_UATHREAD_H_

#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <opcua_platformdefs.h>

class UaThread
{
public:
    UaThread(): m_running(false), m_finished(false) {}
    virtual ~UaThread()
    {
        if (m_thread.joinable())
            m_thread.join();
    }

    virtual void start()
    {
        m_running = true;
        m_thread = std::thread([this]{
            this->run();
            m_running = false;
            m_finished = true;
        });
    }

    OpcUa_Boolean wait( OpcUa_UInt32 /*time*/ = OpcUa_UInt32_Max )
    {
        if (m_thread.joinable())
            m_thread.join();
        return OpcUa_True;
    }

    OpcUa_Boolean running() const { return m_running; }
    OpcUa_Boolean finished() const { return m_finished; }

    static OpcUa_UInt32 currentThread()
    { return static_cast<OpcUa_UInt32>(std::hash<std::thread::id>()(std::this_thread::get_id())); }

    static void sleep( OpcUa_UInt32 secs )  { std::this_thread::sleep_for(std::chrono::seconds(secs)); }
    static void msleep( OpcUa_UInt32 msecs ){ std::this_thread::sleep_for(std::chrono::milliseconds(msecs)); }
    static void usleep( OpcUa_UInt32 usecs ){ std::this_thread::sleep_for(std::chrono::microseconds(usecs)); }

protected:
    virtual void run() = 0;

private:
    std::thread m_thread;
    std::atomic_bool m_running;
    std::atomic_bool m_finished;
};

#endif /* OPEN62541_COMPAT_INCLUDE_UATHREAD_H_ */
