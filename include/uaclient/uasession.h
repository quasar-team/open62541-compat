/*
 * uasession.h
 *
 *  Created on: 24 Nov 2017
 *      Author: pnikiel
 */

#ifndef INCLUDE_UACLIENT_UASESSION_H_
#define INCLUDE_UACLIENT_UASESSION_H_


namespace UaClientSdk
{

class UaSession
{

    UaStatus read(
            ServiceSettings &           serviceSettings,
            OpcUa_Double                maxAge,
            OpcUa_TimestampsToReturn    timeStamps,
            const UaReadValueIds &      nodesToRead,
            UaDataValues &              values,
            UaDiagnosticInfos &         diagnosticInfos  );
};

}



#endif /* INCLUDE_UACLIENT_UASESSION_H_ */
