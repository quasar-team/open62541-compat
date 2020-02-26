/* © Copyright Piotr Nikiel, CERN, 2019.  All rights not expressly granted are reserved.
 * uaserver.h
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

#ifndef OPEN62541_COMPAT_INCLUDE_UASERVER_H_
#define OPEN62541_COMPAT_INCLUDE_UASERVER_H_

#include <thread>
#include <functional> // for std::function

#include <nodemanagerbase.h>

// forward decls from open62541
struct UA_Server;
struct UA_ServerConfig;

class UaServer
{
public:
    UaServer();
    ~UaServer();

    void setServerConfig(const UaString& configurationFile, const UaString& applicationPath);

    void addNodeManager(NodeManagerBase* pNodeManager);

    void linkRunningFlag (volatile OpcUa_Boolean* flag);

    //! Will start the server (in a separate thread). Non-blocking.
    void start();

    //! Will stop the server, if running. Blocking.
    void stop();


private:
    UA_Server *m_server;
    NodeManagerBase* m_nodeManager;
    std::thread m_open62541_server_thread;

    volatile OpcUa_Boolean* m_runningFlag;

    void runThread();

    unsigned int m_endpointPortNumber;
};

#endif /* OPEN62541_COMPAT_INCLUDE_UASERVER_H_ */
