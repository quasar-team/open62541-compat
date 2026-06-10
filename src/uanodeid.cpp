/*
 * uanodeid.cpp
 *
 *  Created on: 27 Nov 2017
 *      Author: pnikiel
 *
 *  This code used to live in open62541_compat.cpp since the beginning.
 *  Now moved to this separate file for easier maintenance and separation
 *  of concerns.
 */

#include <uanodeid.h>
#include <open62541_compat_common.h>
#include <open62541_compat.h>
#include <boost/lexical_cast.hpp>

UaNodeId::UaNodeId ():
	UaNodeId (0, 0) // similarly to UA-SDK
{
}

UaNodeId::UaNodeId ( const UaString& stringAddress, OpcUa_UInt16 ns)
{
    // TODO: not implemented yet in open62541
    //    UA_NodeId_fromInteger( 2, 2);
    m_impl.namespaceIndex = ns;
    m_impl.identifierType = UA_NODEIDTYPE_STRING;
    UA_StatusCode status = UA_String_copy( stringAddress.impl(), &m_impl.identifier.string );
    if (status != UA_STATUSCODE_GOOD)
        throw alloc_error();
}


UaNodeId::UaNodeId( OpcUa_UInt32 numericAddress, OpcUa_UInt16 ns):
    m_impl( UA_NODEID_NUMERIC( ns, numericAddress ))
{

}

UaNodeId::UaNodeId ( const UaNodeId & other)
{
    UA_NodeId_init( &m_impl );
    UA_StatusCode status = UA_NodeId_copy( other.pimpl(), &this->m_impl );
    if (status != UA_STATUSCODE_GOOD)
        throw alloc_error();
}

/* open62541 data format -> UA-SDK data format conversion */
UaNodeId::UaNodeId ( const UA_NodeId& other)
{
	UA_NodeId_init( &m_impl );
	UA_NodeId_copy( &other, &m_impl );
}

const UaNodeId& UaNodeId::operator=(const UaNodeId & other)
{
    UA_NodeId_clear( &m_impl );
    UA_NodeId_init( &m_impl );
    UA_StatusCode status = UA_NodeId_copy( other.pimpl(), &this->m_impl );
    if (status != UA_STATUSCODE_GOOD)
        throw alloc_error();
    return *this;
}

UaNodeId::~UaNodeId ()
{
    UA_NodeId_clear( &m_impl );
}

IdentifierType UaNodeId::identifierType() const
{
    switch (m_impl.identifierType)
    {
    case UA_NODEIDTYPE_NUMERIC: return IdentifierType::OpcUa_IdentifierType_Numeric;
    case UA_NODEIDTYPE_STRING: return IdentifierType::OpcUa_IdentifierType_String;
    default: OPEN62541_COMPAT_LOG_AND_THROW(std::runtime_error, "not-implemented");
    }
}

UaString UaNodeId::identifierString() const
{
    if (m_impl.identifierType != UA_NODEIDTYPE_STRING)
    	OPEN62541_COMPAT_LOG_AND_THROW(std::runtime_error, "asking for identifierString from a non-string identifier!");
    return UaString( &m_impl.identifier.string );

}

bool UaNodeId::operator==(const UaNodeId& other) const
{
    return UA_NodeId_equal( &m_impl, &other.m_impl );
}

UaString UaNodeId::toString() const
{
    if (identifierType() == IdentifierType::OpcUa_IdentifierType_String)
    {
        return identifierString();
    }
    else if (identifierType() == IdentifierType::OpcUa_IdentifierType_Numeric)
    {
        return UaString(boost::lexical_cast<std::string>(identifierNumeric()).c_str());
    }
    return "identifier-type-unsupported";
}

UaString UaNodeId::toFullString() const
{
    std::string s = "NS" + boost::lexical_cast<std::string>(namespaceIndex()) + "|";
    switch (m_impl.identifierType)
    {
    case UA_NODEIDTYPE_NUMERIC:
        s += "Numeric|" + boost::lexical_cast<std::string>(identifierNumeric());
        break;
    case UA_NODEIDTYPE_STRING:
        s += "String|" + UaString(identifierString()).toUtf8();
        break;
    case UA_NODEIDTYPE_BYTESTRING:
    {
        s += "Opaque|0x";
        char hex[3];
        for (size_t i = 0; i < m_impl.identifier.byteString.length; ++i)
        {
            snprintf(hex, sizeof hex, "%02x", m_impl.identifier.byteString.data[i]);
            s += hex;
        }
        break;
    }
    case UA_NODEIDTYPE_GUID:
    {
        char guid[40];
        const UA_Guid& g = m_impl.identifier.guid;
        snprintf(guid, sizeof guid, "Guid|%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                g.data1, g.data2, g.data3,
                g.data4[0], g.data4[1], g.data4[2], g.data4[3],
                g.data4[4], g.data4[5], g.data4[6], g.data4[7]);
        s += guid;
        break;
    }
    }
    return UaString(s.c_str());
}

void UaNodeId::copyTo( UaNodeId* other) const
{
    *other = *this;
}


void UaNodeId::copyTo( UA_NodeId* other) const
{
    if (!other)
    	OPEN62541_COMPAT_LOG_AND_THROW(std::runtime_error, "passed a nullptr");
    UA_StatusCode status = UA_NodeId_copy( &this->m_impl, other );
    if (status != UA_STATUSCODE_GOOD)
        throw alloc_error();

}


