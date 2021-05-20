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
#include <open62541_compat.h>
#include <logit_logger.h>

#include <LogIt.h>

#ifdef HAS_SERVERCONFIG_LOADER
#include <ServerConfig.hxx>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#endif // HAS_SERVERCONFIG_LOADER


UaServer::UaServer() :
m_server(nullptr),
m_nodeManager(nullptr),
m_runningFlag(nullptr),
m_endpointPortNumber(4841)
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
        OPEN62541_COMPAT_LOG_AND_THROW(std::runtime_error, "UA_Server_new failed");
    UA_ServerConfig* config = UA_Server_getConfig(m_server);
    UA_ServerConfig_setMinimal(config, m_endpointPortNumber, nullptr);

	// use LogIt logger for open62541
    initializeOpen62541LogIt();
    config->logger = theLogItLogger;

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
#ifndef HAS_SERVERCONFIG_LOADER
    LOG(Log::INF) << "Note: you built open62541-compat without configuration loading (option SERVERCONFIG_LOADER). So loading of ServerConfig.xml is not supported. Assuming hardcoded server settings (endpoint's port, etc.)";
    //! With open62541 1.0, it is the UA_Server that holds the config.
#else // HAS_SERVERCONFIG_LOADER is defined, means the user wants the option
    std::unique_ptr< ::ServerConfig::OpcServerConfig > serverConfig;
     try
     {
         serverConfig = ServerConfig::OpcServerConfig_ (configurationFile.toUtf8());
     }
     catch (xsd::cxx::tree::parsing<char> &exception)
     {
         LOG(Log::ERR) << "ServerConfig loader: failed when trying to open the file, with general error message: " << exception.what();
         for( const xsd::cxx::tree::error<char> &error : exception.diagnostics() )
         {
             LOG(Log::ERR) << "ServerConfig: Problem at " << error.id() << ":" << error.line() << ": " << error.message();
         }
         OPEN62541_COMPAT_LOG_AND_THROW(std::runtime_error, "ServerConfig: failed to load ServerConfig. The exact problem description should have been logged.");

     }
     // minimum one endpoint is guaranteed by the XSD, but in case user declared more, refuse to continue
     // TODO: implement multiple endpoints
     const ServerConfig::UaServerConfig& uaServerConfig = serverConfig->UaServerConfig();
     if (uaServerConfig.UaEndpoint().size() > 1)
     {
         OPEN62541_COMPAT_LOG_AND_THROW(std::runtime_error, "No support for multiple UaEndpoints yet, simplify your ServerConfig.xml");
     }
     boost::regex endpointUrlRegex("^opc\\.tcp:\\/\\/\\[NodeName\\]:(?<port>\\d+)$");
     boost::smatch matchResults;
     std::string endpointUrl (uaServerConfig.UaEndpoint()[0].Url() );
     bool matched = boost::regex_match( endpointUrl, matchResults, endpointUrlRegex );
     if (!matched)
         OPEN62541_COMPAT_LOG_AND_THROW(std::runtime_error, "Can't parse UaEndpoint/Url, note it should look like 'opc.tcp://[NodeName]:4841' perhaps with different port number, yours is '"+endpointUrl+"'");
     unsigned int endpointUrlPort = boost::lexical_cast<unsigned int>(matchResults["port"]);
     LOG(Log::INF) << "From your [" << configurationFile.toUtf8() << "] loaded endpoint port number: " << endpointUrlPort;
     m_endpointPortNumber = endpointUrlPort;
#endif
}

void UaServer::stop ()
{
	if (m_open62541_server_thread.joinable()) // if start() was never called, or server failed to start, the thread is not joinable...
		m_open62541_server_thread.join();
    delete m_nodeManager;
    m_nodeManager = nullptr;
    UA_Server_delete(m_server);
    m_server = nullptr;
}
