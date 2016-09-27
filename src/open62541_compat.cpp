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
#include <Utils.h>
//#include <ASUtils.h>

class alloc_error: public std::runtime_error
{
public:
    alloc_error(): std::runtime_error("memory allocation exception") {}
};
				 

UaString::UaString ()
{
    m_impl = UA_String_new( );
    if (! m_impl)
	throw alloc_error();

}

UaString::UaString( const char* s)
{
    m_impl = UA_String_new( );
    if ( ! m_impl)
	throw alloc_error();

    *m_impl = UA_String_fromChars( s );
    if( m_impl->length < 0 )
    {
	UA_String_delete( m_impl );
	throw alloc_error();
    }

}

UaString::UaString( const UaString& other)
{
    m_impl = UA_String_new( );
    if ( ! m_impl)
	throw alloc_error();

    if( UA_String_copy( other.m_impl, m_impl ) != UA_STATUSCODE_GOOD)
    {
	UA_String_delete( m_impl );
	throw alloc_error();
    }
   
}

UaString::UaString( const UA_String* other )
{
    m_impl = UA_String_new( );
    if ( ! m_impl)
	throw alloc_error();
    if( UA_String_copy( other, m_impl ) != UA_STATUSCODE_GOOD)
    {
	UA_String_delete( m_impl );
	throw alloc_error();
    }
}    

UaString::~UaString ()
{
    if (m_impl -> data)
	UA_String_deleteMembers( m_impl );
    UA_String_delete( m_impl );
    m_impl = 0;
}

UaString UaString::operator+(const UaString& other)
{
    std::string concatenated = this->toUtf8() + other.toUtf8();
    return UaString( concatenated.c_str() );
}

const UaString& UaString::operator=(const UaString& other)
{
    if (m_impl -> data)
	UA_String_deleteMembers( m_impl );

    if( UA_String_copy( other.m_impl, m_impl ) != UA_STATUSCODE_GOOD)
    {
	UA_String_delete( m_impl );
	throw alloc_error();
    }
    return *this;
}

std::string UaString::toUtf8() const
{
  return std::string( reinterpret_cast<const char*>(m_impl->data), m_impl->length );
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

UA_Variant* UaVariant::createAndCheckOpen6Variant()
{
	UA_Variant* open6Variant = UA_Variant_new();
    if (!open6Variant)
    {
    	throw std::runtime_error("UA_Variant_new() returned 0");
    }
    return open6Variant;
}

void UaVariant::destroyOpen6Variant(UA_Variant* open6Variant)
{
	if(open6Variant)
	{
	    UA_Variant_deleteMembers( open6Variant );
	    UA_Variant_delete( open6Variant );
	}
}

UaVariant::UaVariant ()
:m_impl(createAndCheckOpen6Variant())
{
    LOG(Log::TRC) << __PRETTY_FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( const OpcUa_UInt32& v )
:m_impl(createAndCheckOpen6Variant())
{
    setUInt32( v );
    LOG(Log::TRC) << __PRETTY_FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( const OpcUa_Int32& v )
:m_impl(createAndCheckOpen6Variant())
{
    setInt32( v );
    LOG(Log::TRC) << __PRETTY_FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( const UaString& v )
:m_impl(createAndCheckOpen6Variant())
{
    setString( v );
    LOG(Log::TRC) << __PRETTY_FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( const UaVariant& other)
:m_impl(createAndCheckOpen6Variant())
{
    const UA_StatusCode status = UA_Variant_copy( other.m_impl, this->m_impl );
    if (status != UA_STATUSCODE_GOOD)
    	throw std::runtime_error("UA_Variant_copy failed 0x"+Utils::toHexString(status) );
    LOG(Log::TRC) << __PRETTY_FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}


void UaVariant::operator= (const UaVariant &other)
{
	destroyOpen6Variant(m_impl);
    m_impl = createAndCheckOpen6Variant();
    
    const UA_StatusCode status = UA_Variant_copy( other.m_impl, this->m_impl );
    if (status != UA_STATUSCODE_GOOD)
        throw std::runtime_error("UA_Variant_copy failed 0x"+Utils::toHexString(status) );

    LOG(Log::TRC) << __PRETTY_FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( const UA_Variant& other )
:m_impl(createAndCheckOpen6Variant())
{
    UA_StatusCode status = UA_Variant_copy( &other, this->m_impl );
    if (status != UA_STATUSCODE_GOOD)
        throw std::runtime_error("UA_Variant_copy failed");
}

UaVariant::~UaVariant()
{
    LOG(Log::TRC) <<"+"<< __PRETTY_FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
    destroyOpen6Variant(m_impl);
    m_impl = 0;
}

OpcUaType UaVariant::type() const
{
    if (!m_impl->data)
        return OpcUaType_Null;
    else if(m_impl->type == &UA_TYPES[UA_TYPES_INT16])
        return OpcUaType_Int16;
    else if(m_impl->type == &UA_TYPES[UA_TYPES_UINT16])
        return OpcUaType_UInt16;
    else if(m_impl->type == &UA_TYPES[UA_TYPES_INT32])
        return OpcUaType_Int32;
    else if(m_impl->type == &UA_TYPES[UA_TYPES_UINT32])
        return OpcUaType_UInt32;
    else if(m_impl->type == &UA_TYPES[UA_TYPES_FLOAT])
        return OpcUaType_Float;
    else if(m_impl->type == &UA_TYPES[UA_TYPES_DOUBLE])
        return OpcUaType_Double;
    else if(m_impl->type == &UA_TYPES[UA_TYPES_STRING])
        return OpcUaType_String;
    else
        throw std::runtime_error ("not-implemented");
}

void UaVariant::reuseOrRealloc( const UA_DataType* dataType, void* newValue )
{
    if ((m_impl->data != 0) && (m_impl->type == dataType))
    {
        /* No reason to realloc - the data of the same type will fit for sure! */
        // Piotr: not sure if this is safe for const-size data types like String
        UA_copy( newValue, m_impl->data, dataType );
    }
    else
    {
        /* Data type different - have to realloc */
        UA_Variant_deleteMembers( m_impl );
        // TODO throw when failed
        UA_StatusCode status = UA_Variant_setScalarCopy( m_impl, newValue, dataType);
        if (status != UA_STATUSCODE_GOOD)
            throw std::runtime_error("UA_Variant_setScalarCopy failed:"+Utils::toHexString(status));
    }
}

void UaVariant::setBool( OpcUa_Boolean value )
{
    reuseOrRealloc( &UA_TYPES[UA_TYPES_BOOLEAN], &value );
}

void UaVariant::setInt16( OpcUa_Int16 value )
{
    reuseOrRealloc( &UA_TYPES[UA_TYPES_INT16], &value );
}

void UaVariant::setInt32( OpcUa_Int32 value )
{
    reuseOrRealloc( &UA_TYPES[UA_TYPES_INT32], &value );
}

void UaVariant::setUInt32( OpcUa_UInt32 value )
{
    reuseOrRealloc( &UA_TYPES[UA_TYPES_UINT32], &value );
}

void UaVariant::setInt64( OpcUa_Int64 value )
{
    reuseOrRealloc( &UA_TYPES[UA_TYPES_INT64], &value );
}

void UaVariant::setUInt64( OpcUa_UInt64 value )
{ 
    reuseOrRealloc( &UA_TYPES[UA_TYPES_UINT64], &value );
}

void UaVariant::setFloat( OpcUa_Float value )
{
    reuseOrRealloc( &UA_TYPES[UA_TYPES_FLOAT], &value );
}

void UaVariant::setDouble( OpcUa_Double value )
{
    reuseOrRealloc( &UA_TYPES[UA_TYPES_DOUBLE], &value );
}

void UaVariant::setString( const UaString& value )
{
    /* UASTRING isn't a simple type and has to be handled with care. */
    if (m_impl->data != 0)
        UA_Variant_deleteMembers( m_impl );
    /* Now we assume that the variant is empty */
    UA_StatusCode s = UA_Variant_setScalarCopy( m_impl, value.impl(), &UA_TYPES[UA_TYPES_STRING]);
    
    if (s != UA_STATUSCODE_GOOD)
        throw alloc_error();
}

UaStatus UaVariant::toBool( OpcUa_Boolean& out ) const
{
    return toSimpleType( &UA_TYPES[UA_TYPES_BOOLEAN], &out );
}


UaStatus UaVariant::toInt16( OpcUa_Int16& out ) const
{
    return toSimpleType( &UA_TYPES[UA_TYPES_INT16], &out );
}

UaStatus UaVariant::toInt32( OpcUa_Int32& out ) const
{
    return toSimpleType( &UA_TYPES[UA_TYPES_INT32], &out );
}

UaStatus UaVariant::toUInt32( OpcUa_UInt32& out ) const
{
    return toSimpleType( &UA_TYPES[UA_TYPES_UINT32], &out );
}

UaStatus UaVariant::toInt64( OpcUa_Int64& out ) const
{
    return toSimpleType( &UA_TYPES[UA_TYPES_INT64], &out );
}

UaStatus UaVariant::toFloat( OpcUa_Float& out ) const
{
    return toSimpleType( &UA_TYPES[UA_TYPES_FLOAT], &out );
}

UaStatus UaVariant::toDouble( OpcUa_Double& out ) const
{
    return toSimpleType( &UA_TYPES[UA_TYPES_DOUBLE], &out );
}

UaString UaVariant::toString( ) const
{
    if (m_impl->type != &UA_TYPES[UA_TYPES_STRING])
        throw std::runtime_error("not-a-string");
    return UaString( (UA_String*)m_impl->data );
}

template<typename T>
UaStatus UaVariant::toSimpleType( const UA_DataType* dataType, T* out ) const
{
    //TODO: Add case when array
    //TODO: in principle it should be possible to convert i.e. Int16->Int32, or Byte->Int16, or so... but this is not supported yet
    if (!m_impl || !m_impl->data)
    {
    	LOG(Log::DBG) << __PRETTY_FUNCTION__ << " conversion failed, variant is null";
        return OpcUa_Bad;
    }

    if (dataType != m_impl->type)
    {
    	LOG(Log::DBG) << __PRETTY_FUNCTION__ << " conversion failed, target type ["<<dataType<<"] does not match variant type ["<<m_impl->type<<"]";
        return OpcUa_Bad;  // Incompatible data type
    }

    *out = *static_cast<T*>(m_impl->data);
    return OpcUa_Good;
}



UaDateTime UaDateTime:: now()
{
    //TODO
    return UaDateTime();
}

    // UaDataValue::UaDataValue( const UaDataValue& other)
    // {
    //   // TODO
    // }
 
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

UaNode::UaNode (UaNodeId n_id):
    m_nodeId (n_id)
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
        UaNode(nodeId),
        m_browseName( browseNameNameSpaceIndex, name )
    {
        //      std::cout << __PRETTY_FUNCTION__ << std::endl;
        //      std::cout << "nodeId=" << nodeId.toString().toUtf8() << std::endl;
    }


    BaseDataVariableType::BaseDataVariableType(
        const UaNodeId&    nodeId,
        const UaString&    name,
        OpcUa_UInt16       browseNameNameSpaceIndex,
        const UaVariant&   initialValue,
        OpcUa_Byte         accessLevel,
        NodeManagerConfig* pNodeConfig,
        UaMutexRefCounted* pSharedMutex):
        UaNode (nodeId),
        m_browseName( browseNameNameSpaceIndex, name),
        m_currentValue( initialValue, OpcUa_Good, UaDateTime::now(), UaDateTime::now() )
							   
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
