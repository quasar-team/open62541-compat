/*
 * statuscode.cpp
 *
 *  Created on: 21 Jun 2017
 *      Author: pnikiel
 */

#include <statuscode.h>
#include <Utils.h>

#include <boost/lexical_cast.hpp>

UaString UaStatus::toString() const
{
    switch (m_status)
    {
        case OpcUa_BadParentNodeIdInvalid: return "BadParentNodeIdInvalid";
        default: return ("statuscode=0x"+Utils::toHexString(m_status)+" --missing-description--implement-me--").c_str();
    }
}


