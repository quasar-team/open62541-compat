/*
 * statuscode.cpp
 *
 *  Created on: 21 Jun 2017
 *      Author: pnikiel
 */

#include <statuscode.h>
#include <sstream>

template<typename T>
static std::string toHexString (const T t)
{
        std::ostringstream oss;
        oss << std::hex << (unsigned long)t << std::dec;
        return oss.str ();
}


UaString UaStatus::toString() const
{
    switch (m_status)
    {
        case OpcUa_Good: return "Good";
        case OpcUa_Bad: return "Bad";
        case OpcUa_BadParentNodeIdInvalid: return "BadParentNodeIdInvalid";
        case OpcUa_BadDataEncodingInvalid: return "BadDataEncodingInvalid";
        case OpcUa_BadServerNotConnected: return "BadServerNotConnected";
        case OpcUa_BadOutOfRange: return "BadOutOfRange";
        case OpcUa_BadNotImplemented: return "BadNotImplemented";
        case OpcUa_BadInternalError: return "BadInternalError";
        case OpcUa_BadWaitingForInitialData: return "BadWaitingForInitialData";
        default: return ("statuscode=0x"+toHexString(m_status)+" --missing-description--implement-me-in-statuscode.cpp--").c_str();
    }
}


