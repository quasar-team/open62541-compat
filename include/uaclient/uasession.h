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

#include <atomic>
#include <future>

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
    UaString     sApplicationName;
    UaString     sApplicationUri;
    UaString     sProductUri;
    OpcUa_UInt32 internalServiceCallTimeout;
    OpcUa_UInt32 nSecureChannelLifetime;
    OpcUa_Double nSessionTimeout;

    SessionConnectInfo() :
        sApplicationName("OPC-UA client"),
        sApplicationUri("OPC-UA client"),
        sProductUri("OPC-UA client"),
        internalServiceCallTimeout(5000),
		nSecureChannelLifetime(60/*minutes*/ * 60/*seconds per minute*/ * 1000 /* the unit is miliseconds */ ),
		nSessionTimeout(60/*minutes*/ * 60/*seconds per minute*/ * 1000 /* the unit is miliseconds */)
    {}
};

struct CallIn
{
    CallIn(): objectId(0, 0), methodId(0,0) {}

    UaNodeId         objectId;
    UaNodeId         methodId;
    UaVariantArray   inputArguments;
};

struct CallOut
{
    UaStatus          callResult;
    UaStatusCodeArray inputArgumentResults;
    UaDiagnosticInfos inputArgumentDiagnosticInfos;
    UaVariantArray    outputArguments;

};

struct BrowseContext
{
   /* TODO */
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

    UaStatus call(
            ServiceSettings &       serviceSettings,
            const CallIn &          callRequest,
            CallOut &               callResponse
        );

    UaStatus disconnect(
            ServiceSettings &       serviceSettings,
            OpcUa_Boolean           deleteSubscriptions
        );


    UaStatus browse(
            ServiceSettings&         serviceSettings,
            const UaNodeId&          nodeToBrowse,
            const BrowseContext&     browseContext,
            UaByteString&            continuationPoint,
            UaReferenceDescriptions& referenceDescriptions);


private:
    UA_Client           *m_client;
    std::mutex          m_accessMutex; // used to make all UaSession's methods synchronized

#ifdef OPCUA_2603
    void                clientRunIterateThread(void);
    std::future<void>   m_clientRunIterateThreadFuture;
    std::atomic<bool>   m_clientRunIterateToggle;
#endif

};

}



#endif /* INCLUDE_UACLIENT_UASESSION_H_ */
