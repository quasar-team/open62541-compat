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


#include <iostream>

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
std::vector<StatusCodeDescription> UaStatus::s_statusCodeDescriptions {
    	    	{OpcUa_Bad, "GenericBad"},
				{OpcUa_Uncertain, "GenericUncertain"}
    	    };

UaString UaStatus::toString() const
{

	std::string statusCodeDescription = UA_StatusCode_name(m_status);

	if ( statusCodeDescription == "Unknown StatusCode" )
	{
	  auto localStatus = m_status;
	  std::vector<StatusCodeDescription>::iterator it = std::find_if(s_statusCodeDescriptions.begin(), s_statusCodeDescriptions.end(), [localStatus](StatusCodeDescription &d){return d.statusCode==localStatus;});

	  if ( it != s_statusCodeDescriptions.end())
		  statusCodeDescription = it->description;
	  else if ( isBad() )
	  {
		  statusCodeDescription="GenericBad StatusCode family";
	  }
	  else if ( isUncertain() )
	  {
	          statusCodeDescription="GenericUncertain StatusCode family";
	  }
	  else
	          statusCodeDescription="StatusCode was impossible to parse";
     	}

	statusCodeDescription.append(" (0x"+toHexString(m_status)+")");

	return statusCodeDescription.c_str();
}

bool UaStatus::isBad() const
{
    return std::bitset<32>(m_status).test(31); // 31 ? BAD defines start at 0x8000000 (OPC-UA specification).
}

bool UaStatus::isUncertain() const
{
    return std::bitset<32>(m_status).test(30); // 30 ? Uncertain defines start at 0x4000000 (OPC-UA specification).
}

