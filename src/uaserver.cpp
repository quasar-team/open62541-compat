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
    m_server = UA_Server_new();
    if (!m_server)
        throw std::runtime_error("UA_Server_new failed");
    
    // working with open6 server config, see open62541.h:25878 for structure
    // specifically the endpoints (line 16065) are just a string with no extra port spec
    // we use the minimal config and just the port is set, security "none" defaults.
    // Just need to extract the port number from the .ua sdk ServerConfig.xml file and set it here
    UA_ServerConfig* config = UA_Server_getConfig(m_server);
    UA_ServerConfig_setMinimal(config, 4841, nullptr);
    LOG(Log::INF) << "open6 OPCUA server endpoint hardcoded port to 4841";
    
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
    //! With open62541 1.0, it is the UA_Server that holds the config.
}

void UaServer::stop ()
{
    m_open62541_server_thread.join();
    delete m_nodeManager;
    m_nodeManager = nullptr;
    UA_Server_delete(m_server);
    m_server = nullptr;
}
