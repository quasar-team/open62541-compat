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
#define OpcUa_BadNotImplemented UA_STATUSCODE_BADNOTIMPLEMENTED
#define OpcUa_BadDataEncodingInvalid UA_STATUSCODE_BADDATAENCODINGINVALID
#define OpcUa_BadInvalidArgument OpcUa_Bad // FIXME:
#define OpcUa_BadWaitingForInitialData OpcUa_Bad // FIXME:
#define OpcUa_BadOutOfRange OpcUa_Bad // FIXME:
#define OpcUa_BadUserAccessDenied OpcUa_Bad // FIXME:
#define OpcUa_BadCommunicationError OpcUa_Bad // FIXME:
#define OpcUa_BadNotSupported OpcUa_Bad // FIXME:
#define OpcUa_Uncertain OpcUa_Bad // FIXME
#define OpcUa_BadResourceUnavailable OpcUa_Bad // FIXME:
#define OpcUa_BadInternalError UA_STATUSCODE_BADINTERNALERROR

typedef OpcUa_UInt32 OpcUa_StatusCode;

class UaStatus
{
public:
    UaStatus (int s): m_status(s) {} // from status code
UaStatus(): m_status(0x66666) {} // uninitialized
    bool isGood() const { return m_status == 0; } // TODO
    UaString toString() const { return "???"; }

//TODO: check??
    int statusCode() const { return m_status; }
    operator UA_StatusCode() const { return (UA_StatusCode)m_status; }
private:
    int m_status;
};

#endif //__STATUSCODE_H__
