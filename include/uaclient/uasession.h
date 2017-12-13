/* © Copyright Piotr Nikiel, CERN, 2017.  All rights not expressly granted are reserved.
 *  uasession.h
 *
 *  Created on: 24 Nov, 2017
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

#ifndef INCLUDE_UACLIENT_UASESSION_H_
#define INCLUDE_UACLIENT_UASESSION_H_

#include <mutex>

#include <opcua_types.h>
#include <uaclient/uaclientsdk.h>
#include <arrays.h>

// forward-decl
struct UA_Client;

namespace UaClientSdk
{

struct SessionSecurityInfo
{
    // TODO
};

struct SessionConnectInfo
{
    UaString    sApplicationName;
    UaString    sApplicationUri;
    UaString    sProductUri;
};

namespace UaClient
{

enum ServerStatus
{

};

}

class UaSessionCallback
{

};

class UaSession
{
public:
    UaSession ();
    ~UaSession ();

    UaStatus connect(
            const UaString&                endpoint,
            const SessionConnectInfo&      connectInfo,
            const SessionSecurityInfo&     securityInfo,
            UaSessionCallback*             callback);

    UaStatus read(
            ServiceSettings &           serviceSettings,
            OpcUa_Double                maxAge,
            OpcUa_TimestampsToReturn    timeStamps,
            const UaReadValueIds &      nodesToRead,
            UaDataValues &              values,
            UaDiagnosticInfos &         diagnosticInfos  );

    UaStatus write(
            ServiceSettings &       serviceSettings,
            const UaWriteValues &   nodesToWrite,
            UaStatusCodeArray &     results,
            UaDiagnosticInfos &     diagnosticInfos );

    UaStatus disconnect(
            ServiceSettings &       serviceSettings,
            OpcUa_Boolean           deleteSubscriptions
        );

private:
    UA_Client     *m_client;
    std::mutex    m_accessMutex; // used to make all UaSession's methods synchronized
};

}



#endif /* INCLUDE_UACLIENT_UASESSION_H_ */
