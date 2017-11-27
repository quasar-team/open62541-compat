/*
 * uasession.cpp
 *
 *  Created on: 27 Nov 2017
 *      Author: pnikiel
 */

#include <stdexcept>

#include <open62541.h>
#include <open62541_compat_common.h>
#include <uaclient/uasession.h>
#include <uadatavalue.h>
#include <LogIt.h>

namespace UaClientSdk
{

UaSession::UaSession():
        m_client(0)
{

}

UaStatus UaSession::connect(
        const UaString&                endpoint,
        const SessionConnectInfo&      connectInfo,
        const SessionSecurityInfo&     securityInfo,
        UaSessionCallback*             callback)
{

    if (m_client)
        return OpcUa_Bad;  // Already connected!
    m_client = UA_Client_new(UA_ClientConfig_standard);
    if (!m_client)
        throw alloc_error();
    UaStatus status = UA_Client_connect(m_client, endpoint.toUtf8().c_str());
    if(! status.isGood())
    {
        UA_Client_delete(m_client);
        LOG(Log::ERR) << "in UaSession::connect() " << status.toString().toUtf8();
        return status;
    }
    return OpcUa_Good;
}

/**
 * What's not implemented:
 * - diagnostic infos (TODO)
 * - taking into account serviceSettings (TODO)
 * - type of timestamps (TODO)
 * - maxAge (TODO)
 */
UaStatus UaSession::read(
            ServiceSettings &           serviceSettings,
            OpcUa_Double                maxAge,
            OpcUa_TimestampsToReturn    timeStamps,
            const UaReadValueIds &      nodesToRead,
            UaDataValues &              values,
            UaDiagnosticInfos &         diagnosticInfos  )
{
    if (nodesToRead.size() != 1)
    {
        throw std::runtime_error("So far only single reads are supported"); // FIXME
    }

    if (nodesToRead.size() != values.size())
        throw std::runtime_error("Size of provided values must match size of nodesToRead");

    UA_ReadRequest readRequest;
    UA_ReadRequest_init(&readRequest);
    readRequest.nodesToRead =  (UA_ReadValueId*) UA_Array_new(1, &UA_TYPES[UA_TYPES_READVALUEID]);
    readRequest.nodesToReadSize = 1;
    readRequest.nodesToRead[0].nodeId = nodesToRead[0].NodeId.impl(); // TODO: will it free it or we should free it?
    readRequest.nodesToRead[0].attributeId = nodesToRead[0].AttributeId;

    UA_ReadResponse readResponse = UA_Client_Service_read(m_client, readRequest);
    UaStatus serviceStatus = UaStatus(readResponse.responseHeader.serviceResult);

    if ( serviceStatus.isGood() )
    {
        if (readResponse.resultsSize != nodesToRead.size())
        {
            LOG(Log::ERR) << "mismatch between requested size and returned size.";
            UA_ReadRequest_deleteMembers(&readRequest);
            return OpcUa_Bad;
        }
        for (size_t i=0; i<nodesToRead.size(); ++i)
        {
            if (readResponse.results[i].hasValue)
            {
                //values[i].Value = readResponse.results[i].value;
            }
            if (readResponse.results[i].hasStatus)
            {
                //values[i].StatusCode = readResponse.results[i].status;
            }
            else
            {
                values[i].StatusCode = OpcUa_Good;
                LOG(Log::WRN) << "read response carries no statuscode - somewhat fishy!";
            }
            // TODO: timestamps etc
            // TODO: free the read response
            UA_DataValue_deleteMembers( &readResponse.results[i] );
        }
    }
    UA_ReadRequest_deleteMembers(&readRequest);
    return serviceStatus;

}


}
