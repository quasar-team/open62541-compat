/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
 * open62541_compat.cpp
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

#include <open62541_compat.h>
#include <iostream>
#include <sstream>
#include <bitset>
#include <Utils.h>
#include <boost/format.hpp>
#include <boost/date_time.hpp>

#include <open62541_compat_common.h>


				 
bool UaStatus::isBad() const
{
	return std::bitset<32>(m_status).test(31); // 31 ? BAD defines start at 0x8000000 (OPC-UA specification).
}




UaQualifiedName::UaQualifiedName(int ns, const UaString& name):
    m_unqualifiedName( name )
{
    m_impl.name = *m_unqualifiedName.impl();
    m_impl.namespaceIndex = ns;
}

UaLocalizedText::UaLocalizedText( const char* locale, const char* text) 
{
    m_impl = UA_LOCALIZEDTEXT_ALLOC( locale, text ); 
}

UaLocalizedText::~UaLocalizedText ()
{
    UA_LocalizedText_deleteMembers( &m_impl );
}


UaDateTime::UaDateTime()
:m_dateTime{0}
{}

UaDateTime::UaDateTime(const UA_DateTime& dateTime)
:m_dateTime(dateTime)
{}

UaDateTime UaDateTime::now()
{
    return UaDateTime(UA_DateTime_now());
}

void UaDateTime::addSecs(int secs)
{
	m_dateTime += (secs * UA_SEC_TO_DATETIME);
}

void UaDateTime::addMilliSecs(int msecs)
{
	m_dateTime += (msecs * UA_MSEC_TO_DATETIME);
}

/**
 * Accepts format
 * "%Y-%m-%dT%H:%M:%S%ZP"
 * e.g. unix epoch: "1970-01-01T00:00:00Z"
 * e.g. open62541 epoch "1601-01-01T00:00:00Z" (i.e. windows epoch)
 */
UaDateTime UaDateTime::fromString(const UaString& dateTimeString)
{
	const std::string stdDateTimeString(dateTimeString.toUtf8());
	std::istringstream ss(stdDateTimeString);

	const static std::string timeFormatString("%Y-%m-%dT%H:%M:%S%ZP");
	static std::locale timeFormatLocale(ss.getloc(), new boost::posix_time::time_input_facet(timeFormatString)); // Not a leak: std::locale deletes facet
	ss.imbue(timeFormatLocale);

	try
	{
		static const boost::posix_time::ptime unixEpoch(boost::gregorian::date(1970, 1, 1));

		if(unixEpoch.is_not_a_date_time())
		{
			throw std::runtime_error("Failed to calculate unix epoch, cannot parse any dates from strings.");
		}

		boost::posix_time::ptime dateTime;
		ss >> dateTime;

		if(dateTime.is_not_a_date_time())
		{
			std::ostringstream err;
			err << "Failed to convert string ["<<stdDateTimeString<<"] to a date, valid format ["<<timeFormatString<<"]";
			throw std::runtime_error(err.str());
		}

		const UA_DateTime open62541DateTime = UA_DATETIME_UNIX_EPOCH + ((dateTime - unixEpoch).total_seconds() * UA_SEC_TO_DATETIME);
		return UaDateTime(open62541DateTime);
	}
	catch(const std::runtime_error& e)
	{
		std::ostringstream err;
		err << "Failed to convert string ["<<stdDateTimeString<<"] to a date, valid format ["<<timeFormatString<<"], error: "<<e.what();
		throw std::runtime_error(err.str());
	}
	catch(...)
	{
		std::ostringstream err;
		err << "Failed to convert string ["<<stdDateTimeString<<"] to a date, valid format ["<<timeFormatString<<"], unknown error";
		throw std::runtime_error(err.str());
	}
}

/**
 * Returns format
 * "%Y-%m-%dT%H:%M:%S%ZP"
 * e.g. unix epoch: "1970-01-01T00:00:00Z"
 * e.g. open62541 epoch "1601-01-01T00:00:00Z" (i.e. windows epoch)
 */
UaString UaDateTime::toString() const
{
	const UA_DateTimeStruct dateTime = UA_DateTime_toStruct(m_dateTime);
	std::ostringstream result;

	const double totalNanoSeconds = (dateTime.milliSec * std::pow(10,6)) + (dateTime.microSec * std::pow(10,3)) + (dateTime.nanoSec);
	const double fractionalSeconds = dateTime.sec + (totalNanoSeconds * std::pow(10,-9));
	result << (boost::format("%04d-%02d-%02d:%02d:%02d:%02.09f") % dateTime.year % dateTime.month % dateTime.day %dateTime.hour % dateTime.min % fractionalSeconds);

	return UaString(result.str().c_str());
}

UaDataValue::UaDataValue( const UaVariant& variant, OpcUa_StatusCode statusCode, const UaDateTime& serverTime, const UaDateTime& sourceTime ):
    m_lock( ATOMIC_FLAG_INIT )

{
    m_impl = UA_DataValue_new ();
    if (!m_impl)
        throw std::runtime_error( "UA_DataValue_new returned 0" );
    // LOG(Log::INF) << "allocated new UA_DataValue @ " <<  m_impl;  

    // TODO: duplicate the variant
    UA_Variant_copy( variant.impl(), &m_impl->value );
    LOG(Log::TRC) << "After UA_Variant_copy: src="<<variant.impl()<<" src.data="<<variant.impl()->data<<" dst="<<&m_impl->value<<" dst.data="<<m_impl->value.data;

    m_impl->hasValue = 1;

}

UaDataValue::UaDataValue( const UaDataValue& other ):
    m_lock( ATOMIC_FLAG_INIT )
{
    m_impl = UA_DataValue_new ();
    // LOG(Log::INF) << "allocated new UA_DataValue @ " <<  m_impl;  
    UA_DataValue_copy( other.m_impl, m_impl );
}

void UaDataValue:: operator=(const UaDataValue& other )
{
    while (m_lock.test_and_set(std::memory_order_acquire));  // acquire lock
    if (m_impl)
    {
        UA_DataValue_deleteMembers (m_impl);
        UA_DataValue_delete( m_impl );
        m_impl = 0;
    }
    m_impl = UA_DataValue_new ();
    // LOG(Log::INF) << "allocated new UA_DataValue @ " <<  m_impl;  
    UA_DataValue_copy( other.m_impl, m_impl );
    m_lock.clear(std::memory_order_release);
    
}

UaDataValue UaDataValue::clone()
{
    while (m_lock.test_and_set(std::memory_order_acquire));  // acquire lock
    UaDataValue aCopy ( *this );
    m_lock.clear(std::memory_order_release);
    return aCopy;
    
}

UaDataValue:: ~UaDataValue ()
{
    if (m_impl)
    {
	UA_DataValue_deleteMembers( m_impl );
	UA_DataValue_delete( m_impl );
	m_impl = 0;
    }
    
}

UaNodeId::UaNodeId ( const UaString& stringAddress, int ns)
{
    // TODO: not implemented yet in open62541
    //    UA_NodeId_fromInteger( 2, 2);
    m_impl.namespaceIndex = ns;
    m_impl.identifierType = UA_NODEIDTYPE_STRING;
    UA_StatusCode status = UA_String_copy( stringAddress.impl(), &m_impl.identifier.string );
    if (status != UA_STATUSCODE_GOOD)
        throw alloc_error();
}


UaNodeId::UaNodeId( int numericAddress, int ns ):
    m_impl( UA_NODEID_NUMERIC( ns, numericAddress ))
{
    
}

UaNodeId::UaNodeId ( const UaNodeId& other)
{
    UA_NodeId_init( &m_impl );
    UA_StatusCode status = UA_NodeId_copy( other.pimpl(), &this->m_impl );
    if (status != UA_STATUSCODE_GOOD)
        throw alloc_error();

}

UaNodeId::~UaNodeId ()
{
    UA_NodeId_deleteMembers( &m_impl );
}

IdentifierType UaNodeId::identifierType() const
{
    switch (m_impl.identifierType)
    {
    case UA_NODEIDTYPE_NUMERIC: return IdentifierType::OpcUa_IdentifierType_Numeric;
    case UA_NODEIDTYPE_STRING: return IdentifierType::OpcUa_IdentifierType_String; 
    default: throw std::runtime_error("not-implemented");
    }
}

UaString UaNodeId::identifierString() const 
{
    if (m_impl.identifierType != UA_NODEIDTYPE_STRING)
        throw std::runtime_error("asking for identifierString from a non-string identifier!");
    return UaString( &m_impl.identifier.string );

}	     

bool UaNodeId::operator==(const UaNodeId& other) const
{
    return UA_NodeId_equal( &m_impl, &other.m_impl );
}

UaNode::UaNode ()
{
}

UaString UaNodeId::toString() const
{
    if (identifierType() == IdentifierType::OpcUa_IdentifierType_String)
    {
        std::string s = "(ns="+Utils::toString(namespaceIndex())+","+UaString(identifierString()).toUtf8()+")";
        return UaString(s.c_str());
    }
    else if (identifierType() == IdentifierType::OpcUa_IdentifierType_Numeric)
    {
        std::string s = "(ns="+Utils::toString(namespaceIndex())+","+Utils::toString(identifierNumeric())+")";
        return UaString(s.c_str());
    }
    return "non-string-id";
}

namespace OpcUa
{

    BaseObjectType::BaseObjectType(
        const UaNodeId& nodeId,
        const UaString& name,
        OpcUa_UInt16  browseNameNameSpaceIndex, 
        NodeManagerConfig *nm
        ):
        m_nodeId(nodeId),
        m_browseName( browseNameNameSpaceIndex, name )
    {
        //      std::cout << __PRETTY_FUNCTION__ << std::endl;
        //      std::cout << "nodeId=" << nodeId.toString().toUtf8() << std::endl;
    }

	BaseMethod::BaseMethod (
			const UaNodeId &nodeId,
			const UaString &name,
			OpcUa_UInt16 browseNameNameSpaceIndex,
			UaMutexRefCounted *pSharedMutex):
				m_nodeId(nodeId),
				m_browseName(browseNameNameSpaceIndex, name)

	{

	}


    BaseDataVariableType::BaseDataVariableType(
        const UaNodeId&    nodeId,
        const UaString&    name,
        OpcUa_UInt16       browseNameNameSpaceIndex,
        const UaVariant&   initialValue,
        OpcUa_Byte         accessLevel,
        NodeManagerConfig* pNodeConfig,
        UaMutexRefCounted* pSharedMutex):

        m_browseName( browseNameNameSpaceIndex, name),
        m_currentValue( initialValue, OpcUa_Good, UaDateTime::now(), UaDateTime::now() ),
		m_nodeId (nodeId)
							   
    {
        //      std::cout << __PRETTY_FUNCTION__ <<" (nodeId="<<nodeId.toString().toUtf8()<<")" << std::endl;
    }


    UaStatus BaseDataVariableType::setValue( 
        Session *session,
        const UaDataValue& dataValue,
        OpcUa_Boolean checkAccessLevel
        )
    {
        // TODO check write mask
        // TODO
        m_currentValue = dataValue;
        return OpcUa_Good;
    }

    UaDataValue BaseDataVariableType::value(Session* session) 
    {
        return m_currentValue.clone();
    }
};
