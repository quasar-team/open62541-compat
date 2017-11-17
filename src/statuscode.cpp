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
        case OpcUa_BadParentNodeIdInvalid: return "BadParentNodeIdInvalid";
        default: return ("statuscode=0x"+toHexString(m_status)+" --missing-description--implement-me-in-statuscode.cpp--").c_str();
    }
}


