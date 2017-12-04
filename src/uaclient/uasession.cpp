/* © Copyright Piotr Nikiel, CERN, 2017.  All rights not expressly granted are reserved.
 *  uasession.cpp
 *
 *  Created on: 27 Nov, 2017
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
 *
 * also: this code is highlt non-reentrant because of open62541- TODO: mutex it!
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

    UA_NodeId_init(&readRequest.nodesToRead[0].nodeId);
    nodesToRead[0].NodeId.copyTo( &readRequest.nodesToRead[0].nodeId );

    readRequest.nodesToRead[0].attributeId = nodesToRead[0].AttributeId;

    UA_ReadResponse readResponse = UA_Client_Service_read(m_client, readRequest);

    UA_Array_delete(readRequest.nodesToRead, 1, &UA_TYPES[UA_TYPES_READVALUEID]);

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
                values[i].Value = readResponse.results[i].value;
            }
            if (readResponse.results[i].hasStatus)
            {
                values[i].StatusCode = readResponse.results[i].status;
            }
            else
            {
                values[i].StatusCode = OpcUa_Good;
                LOG(Log::WRN) << "read response carries no statuscode - somewhat fishy!";
            }
            if (readResponse.results[i].hasSourceTimestamp)
            {
                values[i].SourceTimestamp = readResponse.results[i].sourceTimestamp;
            }
            // TODO: timestamps etc
            // TODO: free the read response
        }
    }

    UA_Array_delete(readResponse.results, readResponse.resultsSize, &UA_TYPES[UA_TYPES_DATAVALUE]);

    return serviceStatus;

}

UaStatus UaSession::write(
        ServiceSettings &       serviceSettings,
        const UaWriteValues &   nodesToWrite,
        UaStatusCodeArray &     results,
        UaDiagnosticInfos &     diagnosticInfos )
{
    if (nodesToWrite.size() != 1)
        throw std::runtime_error("so far only implemented for single writes");

    UA_WriteRequest writeRequest;
    UA_WriteRequest_init( &writeRequest);

    writeRequest.nodesToWriteSize = nodesToWrite.size();
    writeRequest.nodesToWrite = (UA_WriteValue*) UA_Array_new(nodesToWrite.size(), &UA_TYPES[UA_TYPES_WRITEVALUE]);

    for (size_t i = 0; i<nodesToWrite.size(); ++i)
    {
        UA_NodeId_init( &writeRequest.nodesToWrite[i].nodeId );
        nodesToWrite[i].NodeId.copyTo( &writeRequest.nodesToWrite[i].nodeId );
        writeRequest.nodesToWrite[i].attributeId = nodesToWrite[i].AttributeId;

        UA_DataValue_init(&writeRequest.nodesToWrite[i].value);
        nodesToWrite[i].Value.Value.copyTo( &writeRequest.nodesToWrite[i].value.value );
        writeRequest.nodesToWrite[i].value.hasValue = UA_TRUE;
    }


    UA_WriteResponse writeResponse = UA_Client_Service_write(m_client, writeRequest);
    UA_Array_delete(writeRequest.nodesToWrite, writeRequest.nodesToWriteSize, &UA_TYPES[UA_TYPES_WRITEVALUE]);

    results.create( nodesToWrite.size() );
    for (size_t i=0; i<nodesToWrite.size(); ++i)
    {
        results[i] = writeResponse.results[i];
    }

    UA_Array_delete(writeResponse.results, writeResponse.resultsSize, &UA_TYPES[UA_TYPES_STATUSCODE]);

    return writeResponse.responseHeader.serviceResult;
}


}
