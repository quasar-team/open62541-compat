/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
 *
 *  Created on: 15 Nov,  2015
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



#ifndef __STATUSCODE_H__
#define __STATUSCODE_H__

#include <open62541.h>
#include <uastring.h>
#include <opcua_platformdefs.h>

#define OpcUa_Good UA_STATUSCODE_GOOD
#define OpcUa_Bad  0x80000000
#define OpcUa_Uncertain 0x40000000
#define OpcUa_BadNotImplemented UA_STATUSCODE_BADNOTIMPLEMENTED
#define OpcUa_BadNoData UA_STATUSCODE_BADNODATA
#define OpcUa_BadDataEncodingInvalid UA_STATUSCODE_BADDATAENCODINGINVALID
#define OpcUa_BadDataUnavailable UA_STATUSCODE_BADDATAUNAVAILABLE
#define OpcUa_BadInvalidArgument UA_STATUSCODE_BADINVALIDARGUMENT
#define OpcUa_BadWaitingForInitialData UA_STATUSCODE_BADWAITINGFORINITIALDATA
#define OpcUa_BadOutOfRange UA_STATUSCODE_BADOUTOFRANGE
#define OpcUa_BadUserAccessDenied UA_STATUSCODE_BADUSERACCESSDENIED
#define OpcUa_BadNoCommunication UA_STATUSCODE_BADNOCOMMUNICATION
#define OpcUa_BadCommunicationError UA_STATUSCODE_BADCOMMUNICATIONERROR
#define OpcUa_BadNotSupported UA_STATUSCODE_BADNOTSUPPORTED
#define OpcUa_BadResourceUnavailable UA_STATUSCODE_BADRESOURCEUNAVAILABLE
#define OpcUa_BadInternalError UA_STATUSCODE_BADINTERNALERROR
#define OpcUa_BadInvalidState UA_STATUSCODE_BADINVALIDSTATE
#define OpcUa_UncertainInitialValue UA_STATUSCODE_UNCERTAININITIALVALUE
#define OpcUa_BadUnexpectedError UA_STATUSCODE_BADUNEXPECTEDERROR
#define OpcUa_BadParentNodeIdInvalid UA_STATUSCODE_BADPARENTNODEIDINVALID

typedef OpcUa_UInt32 OpcUa_StatusCode;

class UaStatus
{
public:
    UaStatus (OpcUa_StatusCode s): m_status(s) {} // from status code
UaStatus(): m_status(0x66666) {} // uninitialized
    bool isGood() const { return m_status == UA_STATUSCODE_GOOD; }
    bool isNotGood() const { return !isGood(); }
    bool isBad() const;
    UaString toString() const;

//TODO: check??
    int statusCode() const { return m_status; }
    operator UA_StatusCode() const { return (UA_StatusCode)m_status; }
private:
    OpcUa_StatusCode m_status;
};

#endif //__STATUSCODE_H__
