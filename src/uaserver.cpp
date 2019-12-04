/* © Copyright Piotr Nikiel, CERN, 2019.  All rights not expressly granted are reserved.
 * uaserver.cpp
 *
 *  Created on: 29 Nov 2019
 *      Author: Piotr Nikiel <piotr@nikiel.info>
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

#include <stdexcept>

#include <open62541.h>

#include <uaserver.h>
#include <statuscode.h>

#include <LogIt.h>

#define OPEN62541_COMPAT_LOG_AND_THROW(EXCEPTION_TYPE, MSG) \
    { \
    LOG(Log::ERR) << MSG; \
    throw EXCEPTION_TYPE (MSG); \
    }

UaServer::UaServer() :
m_server(nullptr),
m_nodeManager(nullptr),
m_server_config(nullptr, &UA_ServerConfig_delete),
m_runningFlag(nullptr)
{

}

UaServer::~UaServer()
{
    // TODO: what if still running?
}

void UaServer::start()
{
    if (!m_runningFlag)
        throw std::logic_error ("Establish the 'running flag' first");
    if (!m_server_config)
        throw std::logic_error ("Server must be provisioned with the configuration, call setServerConfig first");
    m_server = UA_Server_new(m_server_config.get());
    if (!m_server)
        throw std::runtime_error("UA_Server_new failed");

    m_nodeManager->linkServer(m_server);
    m_nodeManager->afterStartUp();

    UA_StatusCode status = UA_Server_run_startup(m_server);
    if (status != UA_STATUSCODE_GOOD)
        throw std::runtime_error("UA_Server_run_startup returned not-good, server can't start. Error was:"+
                UaStatus(status).toString().toUtf8());
    else
        LOG(Log::INF) <<
        "UA_Server_run_startup returned: " << UaStatus(status).toString().toUtf8() << ", continuing.";
    m_open62541_server_thread = std::thread ( &UaServer::runThread, this );
}

void UaServer::runThread()
{
    while (*m_runningFlag)
    {
        UA_Server_run_iterate(m_server, true);
    }
    UA_StatusCode status = UA_Server_run_shutdown(m_server);
    if (status != UA_STATUSCODE_GOOD)
    {
        LOG(Log::ERR) << "UA_Server_run_shutdown returned not-good. Error was:" << UaStatus(status).toString().toUtf8();
    }
    else
        LOG(Log::INF) << "UA_Server_run_shutdown returned: " << UaStatus(status).toString().toUtf8();
}

void UaServer::addNodeManager(NodeManagerBase* pNodeManager)
{
    if (!m_nodeManager)
        m_nodeManager = pNodeManager;
    else
        OPEN62541_COMPAT_LOG_AND_THROW(std::logic_error, "Sorry, only 1 NodeManager is supported.");
}

void UaServer::linkRunningFlag (volatile OpcUa_Boolean* flag)
{
    m_runningFlag = flag;
}

void UaServer::setServerConfig(
        const UaString& configurationFile,
        const UaString& applicationPath)
{
    LOG(Log::INF) << "Note: with open62541 backend, there isn't (yet) XML configuration loading. Assuming hardcoded server settings (endpoint's port, etc.)";
    m_server_config.reset( UA_ServerConfig_new_minimal(4841, /*certificate*/ nullptr) );
}

void UaServer::stop ()
{
    m_open62541_server_thread.join();
    delete m_nodeManager;
    m_nodeManager = nullptr;
    UA_Server_delete(m_server);
    m_server = nullptr;
}