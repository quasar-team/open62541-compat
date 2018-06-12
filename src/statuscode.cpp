/*
 * statuscode.cpp
 *
 *  Created on: 21 Jun 2017
 *      Author: pnikiel, pmoschov, bfarnham
 */

#include <statuscode.h>
#include <sstream>
#include <string>
#include <bitset>

template<typename T>
static std::string toHexString (const T t)
{
        std::ostringstream oss;
        oss << std::hex << (unsigned long)t << std::dec;
        return oss.str ();
}


UaString UaStatus::toString() const
{

	std::string buf(UA_StatusCode_name(m_status));
	buf.append(": ");
	buf.append(UA_StatusCode_explanation(m_status));
	buf.append(" StatusCode (0x"+toHexString(m_status)+")");

	return buf.c_str();

}

bool UaStatus::isBad() const
{
    return std::bitset<32>(m_status).test(31); // 31 ? BAD defines start at 0x8000000 (OPC-UA specification).
}
