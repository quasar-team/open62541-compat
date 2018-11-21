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
#include <algorithm>
#include <vector>

template<typename T>
static std::string toHexString (const T t)
{
        std::ostringstream oss;
        oss << std::hex << (unsigned long)t << std::dec;
        return oss.str ();
}

/*
 * Stores the description of status codes that do not exist in open62541 and are used by quasar.
 */
std::vector<statusCodeDescription> statusCodeDescriptions {
    	    	{OpcUa_Bad, "GenericBad"},
    	    	{OpcUa_Uncertain, "GenericUncertain"}
    	    };

UaString UaStatus::toString() const
{
	std::string buf(statusCodeName());

	if (buf=="Unknown StatusCode" && isBad())
	{
		buf.clear();
		buf.append("GenericBad StatusCode family");
	}
	else if (buf=="Unknown StatusCode" && isUncertain())
	{
		buf.clear();
		buf.append("GenericUncertain StatusCode family");
	}

	buf.append(" (0x"+toHexString(m_status)+")");

	return buf.c_str();
}

std::string UaStatus::statusCodeName() const
{

	std::vector<statusCodeDescription>::iterator it;
	std::string statusCodeDesctription = UA_StatusCode_name(m_status);

	if ( statusCodeDesctription == "Unknown StatusCode" )
	{
		it = std::find_if(
				statusCodeDescriptions.begin(),
				statusCodeDescriptions.end(),
				findStatusCode( m_status ));
	}

	if ( (it - statusCodeDescriptions.begin()) < statusCodeDescriptions.size() )
		return statusCodeDescriptions[it - statusCodeDescriptions.begin()].description;

	return statusCodeDesctription;
}

bool UaStatus::isBad() const
{
    return std::bitset<32>(m_status).test(31); // 31 ? BAD defines start at 0x8000000 (OPC-UA specification).
}

bool UaStatus::isUncertain() const
{
    return std::bitset<32>(m_status).test(30); // 30 ? Uncertain defines start at 0x4000000 (OPC-UA specification).
}

