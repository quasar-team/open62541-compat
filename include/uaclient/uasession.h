/*
 * uasession.h
 *
 *  Created on: 24 Nov 2017
 *      Author: pnikiel
 */

#ifndef INCLUDE_UACLIENT_UASESSION_H_
#define INCLUDE_UACLIENT_UASESSION_H_


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

private:
    UA_Client *m_client;
};

}



#endif /* INCLUDE_UACLIENT_UASESSION_H_ */
