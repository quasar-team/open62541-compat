/*
 * uaclientsdk.h
 *
 *  Created on: 24 Nov 2017
 *      Author: pnikiel
 */

#ifndef OPEN62541_COMPAT_INCLUDE_UACLIENT_UACLIENTSDK_H_
#define OPEN62541_COMPAT_INCLUDE_UACLIENT_UACLIENTSDK_H_


#include <uastring.h>

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

}



#endif /* OPEN62541_COMPAT_INCLUDE_UACLIENT_UACLIENTSDK_H_ */
