/*
 * uasession.cpp
 *
 *  Created on: 27 Nov 2017
 *      Author: pnikiel
 */

#include <uaclient/uasession.h>

namespace UaClientSdk
{

UaStatus UaSession::connect(
        const UaString&                endpoint,
        const SessionConnectInfo&      connectInfo,
        const SessionSecurityInfo&     securityInfo,
        UaSessionCallback*             callback)
{

    return OpcUa_BadNotImplemented;
}

UaStatus UaSession::read(
            ServiceSettings &           serviceSettings,
            OpcUa_Double                maxAge,
            OpcUa_TimestampsToReturn    timeStamps,
            const UaReadValueIds &      nodesToRead,
            UaDataValues &              values,
            UaDiagnosticInfos &         diagnosticInfos  )
{
    return OpcUa_BadNotImplemented;

}


}
