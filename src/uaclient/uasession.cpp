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

#include <boost/lexical_cast.hpp>

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

UaSession::~UaSession ()
{
    if (m_client)
    {
        ServiceSettings standardServiceSettings;
        this->disconnect(
                standardServiceSettings,
                /*delete subscriptions*/ OpcUa_True);
    }

}

UaStatus UaSession::connect(
        const UaString&                endpoint,
        const SessionConnectInfo&      connectInfo,
        const SessionSecurityInfo&     securityInfo,
        UaSessionCallback*             callback)
{
    std::lock_guard<decltype(m_accessMutex)> lock (m_accessMutex);

    if (m_client)
    {
        LOG(Log::ERR) << "Connection already exists, can't call connect()";
        return OpcUa_Bad;  // Already connected!
    }
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
 * TODO: serviceSettings,
 * TODO: deleteSubscriptions
 */
UaStatus UaSession::disconnect(
        ServiceSettings &       serviceSettings,
        OpcUa_Boolean           deleteSubscriptions
    )
{
    std::lock_guard<decltype(m_accessMutex)> lock (m_accessMutex);
    if (! m_client)
    {
        LOG(Log::WRN) << "Can't disconnect because not connected.";
        return OpcUa_BadInvalidState;  // can't disconnect if not connected...
    }
    UaStatus status = UA_Client_disconnect(m_client);
    if (m_client)
    {
        UA_Client_delete(m_client);
        m_client = 0;
    }
    return status;
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
    std::lock_guard<decltype(m_accessMutex)> lock (m_accessMutex);
    if (nodesToRead.size() != 1)
    {

        throw std::runtime_error("UaSession::read(): So far only single reads are supported, but you requested a read of "
                +boost::lexical_cast<std::string>(nodesToRead.size())+" items. FIXME!");
        // FIXME:implement this
    }

    LOG(Log::TRC) << "UaSession::read( nodesToRead=[" << nodesToRead[0].NodeId.toString().toUtf8() << "] )";

    if (nodesToRead.size() != values.size())
        throw std::runtime_error("Size of provided value holders (is "
                +boost::lexical_cast<std::string>(values.size())
                +" must match size of provided nodesToRead (is "
                +boost::lexical_cast<std::string>(nodesToRead.size()));

    UA_ReadRequest readRequest;
    UA_ReadRequest_init(&readRequest);
    readRequest.nodesToRead =  (UA_ReadValueId*) UA_Array_new(1, &UA_TYPES[UA_TYPES_READVALUEID]);
    readRequest.nodesToReadSize = 1;
    // The following should be safe because enum values are defined with open62541 defines
    readRequest.timestampsToReturn = (UA_TimestampsToReturn)timeStamps;
    readRequest.maxAge = maxAge;

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
            LOG(Log::ERR) << "after call to open62541: mismatch between requested size "
                    << boost::lexical_cast<std::string>(readRequest.nodesToReadSize)
                    << " and returned size "
                    << boost::lexical_cast<std::string>(readResponse.resultsSize);
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
            if (readResponse.results[i].hasServerTimestamp)
            {
                values[i].ServerTimestamp = readResponse.results[i].serverTimestamp;
            }
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
    std::lock_guard<decltype(m_accessMutex)> lock (m_accessMutex);
    if (nodesToWrite.size() != 1)
    {
        throw std::runtime_error("UaSession::write(): So far only single writes are supported, but you requested a write of "
                +boost::lexical_cast<std::string>(nodesToWrite.size())+" items. FIXME!");
        // FIXME:implement this
    }

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
