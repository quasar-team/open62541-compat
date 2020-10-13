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
#include <managed_uaarray.h>

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
    m_client = UA_Client_new();
    if (!m_client)
        throw alloc_error();
    UA_ClientConfig* clientConfig = UA_Client_getConfig(m_client);
    UA_ClientConfig_setDefault(clientConfig);
    clientConfig->timeout = connectInfo.internalServiceCallTimeout;
    // TODO @Piotr note that many possibly important settings are not carried
    // from UA-SDK API to open6! At the moment, only timeout is.

    UaStatus status = UA_Client_connect(m_client, endpoint.toUtf8().c_str());
    if(! status.isGood())
    {
        UA_Client_delete(m_client);
        m_client = nullptr;
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
 *
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
    ManagedUaArray<UA_ReadValueId> readValueIds (1, &UA_TYPES[UA_TYPES_READVALUEID]);
    readRequest.nodesToRead = readValueIds;
    readRequest.nodesToReadSize = 1;
    // The following should be safe because enum values are defined with open62541 defines
    readRequest.timestampsToReturn = (UA_TimestampsToReturn)timeStamps;
    readRequest.maxAge = maxAge;

    UA_NodeId_init(&readRequest.nodesToRead[0].nodeId);
    nodesToRead[0].NodeId.copyTo( &readRequest.nodesToRead[0].nodeId );

    readRequest.nodesToRead[0].attributeId = nodesToRead[0].AttributeId;

    UA_ReadResponse readResponse = UA_Client_Service_read(m_client, readRequest);
    ManagedUaArray<UA_DataValue> readResponseResults( readResponse.resultsSize, &UA_TYPES[UA_TYPES_DATAVALUE], readResponse.results);

    UaStatus serviceStatus = UaStatus(readResponse.responseHeader.serviceResult);

    if ( serviceStatus.isGood() )
    {
        if (readResponse.resultsSize != nodesToRead.size())
        {
            LOG(Log::ERR) << "after call to open62541: mismatch between requested size "
                    << boost::lexical_cast<std::string>(readRequest.nodesToReadSize)
                    << " and returned size "
                    << boost::lexical_cast<std::string>(readResponse.resultsSize);
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
                // Look at OPCUA-938
                // This is actually a safe assumption: when the statuscode is good(0) it is not transmitted on the wire
                values[i].StatusCode = OpcUa_Good;
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
    ManagedUaArray<UA_WriteValue> writeValues (nodesToWrite.size(), &UA_TYPES[UA_TYPES_WRITEVALUE]);
    writeRequest.nodesToWrite = writeValues;

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
    ManagedUaArray<UA_StatusCode> statusCodes( writeResponse.resultsSize, &UA_TYPES[UA_TYPES_STATUSCODE], writeResponse.results );

    if (UaStatus(writeResponse.responseHeader.serviceResult).isGood())
    {
        results.create( writeResponse.resultsSize );
        for (size_t i=0; i<writeResponse.resultsSize; ++i)
        {
            results[i] = writeResponse.results[i];
        }
    }
    else
        results.create(0); // assume there are no results when the service call failed

    return writeResponse.responseHeader.serviceResult;
}


/* What's not implemented:
 * - diagnosticInfos
 */
UaStatus UaSession::call(
        ServiceSettings &       serviceSettings,
        const CallIn &          callIn,
        CallOut &               callOut
    )
{
    std::lock_guard<decltype(m_accessMutex)> lock (m_accessMutex);

    UA_CallMethodRequest methodCallRequest;
    UA_CallMethodRequest_init(&methodCallRequest);
    methodCallRequest.methodId = callIn.methodId.impl();
    methodCallRequest.objectId = callIn.objectId.impl();
    methodCallRequest.inputArgumentsSize = callIn.inputArguments.size();
    ManagedUaArray<UA_Variant> inputArguments (callIn.inputArguments.size(), &UA_TYPES[UA_TYPES_VARIANT]);
    methodCallRequest.inputArguments = inputArguments;

    bool anyArgumentFailed = false;
    for (unsigned int i=0; i<callIn.inputArguments.size(); ++i)
    {
        UA_Variant_init(&methodCallRequest.inputArguments[i]);
        UaStatus status = UA_Variant_copy(callIn.inputArguments[i].impl(), &methodCallRequest.inputArguments[i] );
        if (!status.isGood())
        {
            LOG(Log::ERR) << "UA_Variant_copy said: " << status.toString().toUtf8();
            anyArgumentFailed = true;
        }
    }
    if (anyArgumentFailed)
    { // free all, no point in continuing
        throw std::runtime_error("Failed to copy arguments through UA_Variant_copy, potentially memory issue.");
    }

    UA_CallRequest callRequest;
    UA_CallRequest_init( &callRequest);

    callRequest.methodsToCallSize = 1;
    callRequest.methodsToCall = &methodCallRequest;

    UA_CallResponse callResponse;
    UA_CallResponse_init(&callResponse);
    callResponse = UA_Client_Service_call(m_client, callRequest);
    ManagedUaArray<UA_CallMethodResult> results (  // RAII holder for data allocated in o6, doesn't alloc anything.
            callResponse.resultsSize,
            &UA_TYPES[UA_TYPES_CALLMETHODRESULT],
            callResponse.results );

    if (!UaStatus(callResponse.responseHeader.serviceResult).isGood())
    {
        return callResponse.responseHeader.serviceResult;
    }

    callOut.inputArgumentDiagnosticInfos.create(0);
    callOut.inputArgumentResults.create(0);

    // there should be at least one callResponse result because we called one method
    if (callResponse.resultsSize != 1)
        throw std::logic_error("One method called so expected one call response, but instead got: "+boost::lexical_cast<std::string>(callResponse.resultsSize)+", open62541 error?");

    UA_CallMethodResult *result = &callResponse.results[0];

    callOut.outputArguments.create( result->outputArgumentsSize );
    for (unsigned int i=0; i<result->outputArgumentsSize; ++i)
    {
        callOut.outputArguments[i] = result->outputArguments[i];
    }

    callOut.callResult = callResponse.responseHeader.serviceResult;
    if (callOut.callResult.isGood())
        callOut.callResult = callResponse.results[0].statusCode; // cast individual error as per OPCUA-1198

    return callOut.callResult;

}

UaStatus UaSession::browse(
        ServiceSettings&         serviceSettings,
        const UaNodeId&          nodeToBrowse,
        const BrowseContext&     browseContext,
        UaByteString&            continuationPoint,
        UaReferenceDescriptions& referenceDescriptions)
{
    UA_BrowseDescription browseDescription;
    UA_BrowseDescription_init (&browseDescription);
    nodeToBrowse.copyTo(&browseDescription.nodeId);
    browseDescription.browseDirection = UA_BROWSEDIRECTION_FORWARD; // Piotr: unsure here if we should have both?
    //TODO: certainly we're missing a few fields, at least.

    UA_BrowseRequest browseRequest;
    UA_BrowseRequest_init(&browseRequest);

    browseRequest.nodesToBrowseSize = 1;
    browseRequest.nodesToBrowse = &browseDescription;

    UA_BrowseResponse browseResponse;
    UA_BrowseResponse_init(&browseResponse);

    browseResponse = UA_Client_Service_browse(m_client, browseRequest);
    UaStatus serviceResult (browseResponse.responseHeader.serviceResult);

    if (!serviceResult.isGood())
    {
    	LOG(Log::ERR) << "Error: the open62541 browse call on node " << nodeToBrowse.toString().toUtf8() << " returned bad status: " << serviceResult.toString().toUtf8();
        return serviceResult;
    }

    for (int i=0; i < browseResponse.resultsSize; i++)
    {
      UA_BrowseResult& br = browseResponse.results[i];
      LOG(Log::DBG) << "Browse result " << i << ", references size: " << br.referencesSize;
      for (int j=0; j < br.referencesSize; j++)
      {
        UA_ReferenceDescription& rd = br.references[j];
        LOG(Log::DBG) << "... ref #" << j << ": typeId=" << UaNodeId(rd.referenceTypeId).toString().toUtf8();
      }

    }

    LOG(Log::INF) << "After browse (was OK): resultsSize= " << browseResponse.resultsSize;



    return browseResponse.responseHeader.serviceResult;

}


}
