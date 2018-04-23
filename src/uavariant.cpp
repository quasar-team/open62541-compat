/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
 * uavariant.cpp
 *
 *  Created on: 15 Nov,  2015
 *      Author: Piotr Nikiel <piotr@nikiel.info>
 *      Author: Ben Farnham <benjamin.farnham@cern.ch>
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
#include <boost/format.hpp>
#include <boost/date_time.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <open62541_compat_common.h>
				 

UA_Variant* UaVariant::createAndCheckOpen62541Variant()
{
	UA_Variant* open62541Variant = UA_Variant_new();
    if (!open62541Variant)
    {
    	throw std::runtime_error("UA_Variant_new() returned 0");
    }
    return open62541Variant;
}

void UaVariant::destroyOpen62541Variant(UA_Variant* open62541Variant)
{
	if(open62541Variant)
	{
	    UA_Variant_deleteMembers( open62541Variant );
	    UA_Variant_delete( open62541Variant );
	}
}

UaVariant::UaVariant ()
:m_impl(createAndCheckOpen62541Variant())
{
    LOG(Log::TRC) << __FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( OpcUa_UInt32 v )
:m_impl(createAndCheckOpen62541Variant())
{
    setUInt32( v );
    LOG(Log::TRC) << __FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( OpcUa_Int32 v )
:m_impl(createAndCheckOpen62541Variant())
{
    setInt32( v );
    LOG(Log::TRC) << __FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( OpcUa_UInt64 v )
:m_impl(createAndCheckOpen62541Variant())
{
    setUInt64( v );
    LOG(Log::TRC) << __FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( OpcUa_Int64 v )
:m_impl(createAndCheckOpen62541Variant())
{
    setInt64( v );
    LOG(Log::TRC) << __FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( const UaString& v )
:m_impl(createAndCheckOpen62541Variant())
{
    setString( v );
    LOG(Log::TRC) << __FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( OpcUa_Float v )
:m_impl(createAndCheckOpen62541Variant())
{
	setFloat(v);
	LOG(Log::TRC) << __FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( OpcUa_Double v )
:m_impl(createAndCheckOpen62541Variant())
{
	setDouble(v);
	LOG(Log::TRC) << __FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( OpcUa_Boolean v )
:m_impl(createAndCheckOpen62541Variant())
{
	setBool(v);
	LOG(Log::TRC) << __FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( const UaVariant& other)
:m_impl(createAndCheckOpen62541Variant())
{
    const UaStatus status = UA_Variant_copy( other.m_impl, this->m_impl );
    if (! status.isGood())
      throw std::runtime_error(std::string("UA_Variant_copy failed:") + status.toString().toUtf8() );
    LOG(Log::TRC) << __FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

UaVariant::UaVariant( const UaByteString& v)
:m_impl(createAndCheckOpen62541Variant())
{
    setByteString(v, /*detach*/ false);
}

void UaVariant::operator= (const UaVariant &other)
{
	destroyOpen62541Variant(m_impl);
    m_impl = createAndCheckOpen62541Variant();
    
    const UaStatus status = UA_Variant_copy( other.m_impl, this->m_impl );
    if (! status.isGood())
        throw std::runtime_error(std::string("UA_Variant_copy failed:") + status.toString().toUtf8() );

    LOG(Log::TRC) << __FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
}

bool UaVariant::operator==(const UaVariant& other) const
{
	if(m_impl == 0 || other.m_impl == 0) return false; // uninitialized - cannot compare.
	if(type() != other.type()) return false;
	if(m_impl->arrayLength != other.m_impl->arrayLength) return false;
	if(m_impl->arrayDimensionsSize != other.m_impl->arrayDimensionsSize) return false;
	for(size_t arrayDimensionIndex = 0; arrayDimensionIndex < m_impl->arrayDimensionsSize; ++arrayDimensionIndex)
	{
		if(m_impl->arrayDimensions[arrayDimensionIndex] != other.m_impl->arrayDimensions[arrayDimensionIndex]) return false;
	}
	if(0 != memcmp(m_impl->data, other.m_impl->data, m_impl->arrayLength)) return false;

	return true;
}

UaVariant::UaVariant( const UA_Variant& other )
:m_impl(createAndCheckOpen62541Variant())
{
    UA_StatusCode status = UA_Variant_copy( &other, this->m_impl );
    if (status != UA_STATUSCODE_GOOD)
        throw std::runtime_error("UA_Variant_copy failed");
}

UaVariant::~UaVariant()
{
    LOG(Log::TRC) <<"+"<< __FUNCTION__ << " m_impl="<<m_impl<<" m_impl.data="<<m_impl->data;
    destroyOpen62541Variant(m_impl);
    m_impl = 0;
}

OpcUaType UaVariant::type() const
{
    if (!m_impl->data)
        return OpcUaType_Null;
    else
    {
	if (m_impl->type->typeId.identifierType == UA_NODEIDTYPE_NUMERIC)
	    return static_cast<OpcUaType>( m_impl->type->typeId.identifier.numeric );
	else
	    throw std::runtime_error("No support for non built-in data types in variant! (yet)");
    }
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
        UaStatus status = UA_Variant_setScalarCopy( m_impl, newValue, dataType);
        if (! status.isGood())
	  throw std::runtime_error(std::string("UA_Variant_setScalarCopy failed:")+status.toString().toUtf8());
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

void UaVariant::setUInt16( OpcUa_UInt16 value )
{
    reuseOrRealloc( &UA_TYPES[UA_TYPES_UINT16], &value );
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

void UaVariant::setByteString( const UaByteString& value, bool detach)
{
	if (detach)
		throw std::runtime_error("value detachment not yet implemented");
    if (m_impl->data != 0)
        UA_Variant_deleteMembers( m_impl );
    UA_StatusCode s = UA_Variant_setScalarCopy( m_impl, value.impl(), &UA_TYPES[UA_TYPES_BYTESTRING]);
    if (s != UA_STATUSCODE_GOOD)
        throw alloc_error();
}

template<typename ArrayType>
void UaVariant::set1DArray( const UA_DataType* dataType, const ArrayType& input )
{
    if (m_impl->data != 0)
        UA_Variant_deleteMembers( m_impl );
    const void* rawInput = 0;
    if (input.size()>0)
        rawInput = &input[0];
    if (UA_Variant_setArrayCopy(
            m_impl,
            rawInput,
            input.size(),
            dataType) != UA_STATUSCODE_GOOD)
        throw alloc_error();
}

void UaVariant::setBoolArray(
        UaBooleanArray &    input,
        OpcUa_Boolean       bDetach
    )
{
    if (bDetach) throw std::runtime_error("value detachment not yet implemented");
    set1DArray( &UA_TYPES[UA_TYPES_BOOLEAN], input );
}

void UaVariant::setSByteArray(
        UaSByteArray &      input,
        OpcUa_Boolean       bDetach
    )
{
    if (bDetach) throw std::runtime_error("value detachment not yet implemented");
    set1DArray( &UA_TYPES[UA_TYPES_SBYTE], input );
}

void UaVariant::setByteArray(
        UaByteArray &      input,
        OpcUa_Boolean       bDetach
    )
{
    if (bDetach) throw std::runtime_error("value detachment not yet implemented");
    set1DArray( &UA_TYPES[UA_TYPES_BYTE], input );
}

void UaVariant::setInt16Array(
        UaInt16Array &      input,
        OpcUa_Boolean       bDetach
    )
{
    if (bDetach) throw std::runtime_error("value detachment not yet implemented");
    set1DArray( &UA_TYPES[UA_TYPES_INT16], input );
}

void UaVariant::setUInt16Array(
        UaUInt16Array &      input,
        OpcUa_Boolean       bDetach
    )
{
    if (bDetach) throw std::runtime_error("value detachment not yet implemented");
    set1DArray( &UA_TYPES[UA_TYPES_UINT16], input );
}

void UaVariant::setInt32Array(
        UaInt32Array &      input,
        OpcUa_Boolean       bDetach
    )
{
    if (bDetach) throw std::runtime_error("value detachment not yet implemented");
    set1DArray( &UA_TYPES[UA_TYPES_INT32], input );
}

void UaVariant::setUInt32Array(
        UaUInt32Array &      input,
        OpcUa_Boolean       bDetach
    )
{
    if (bDetach) throw std::runtime_error("value detachment not yet implemented");
    set1DArray( &UA_TYPES[UA_TYPES_UINT32], input );
}

void UaVariant::setInt64Array(
        UaInt64Array &      input,
        OpcUa_Boolean       bDetach
    )
{
    if (bDetach) throw std::runtime_error("value detachment not yet implemented");
    set1DArray( &UA_TYPES[UA_TYPES_INT64], input );
}

void UaVariant::setUInt64Array(
        UaUInt64Array &      input,
        OpcUa_Boolean       bDetach
    )
{
    if (bDetach) throw std::runtime_error("value detachment not yet implemented");
    set1DArray( &UA_TYPES[UA_TYPES_UINT64], input );
}

void UaVariant::setFloatArray(
        UaFloatArray &      input,
        OpcUa_Boolean       bDetach
    )
{
    if (bDetach) throw std::runtime_error("value detachment not yet implemented");
    set1DArray( &UA_TYPES[UA_TYPES_FLOAT], input );
}

void UaVariant::setDoubleArray(
        UaDoubleArray &      input,
        OpcUa_Boolean       bDetach
    )
{
    if (bDetach) throw std::runtime_error("value detachment not yet implemented");
    set1DArray( &UA_TYPES[UA_TYPES_DOUBLE], input );
}
//
void UaVariant::setStringArray(
        UaStringArray &    input,
        OpcUa_Boolean      bDetach
    )
{
    if (bDetach) throw std::runtime_error("value detachment not yet implemented");
    if (m_impl->data != 0)
        UA_Variant_deleteMembers( m_impl );
    /* Hate void* but hey, it seems open62541 way. */
    UA_String* array = static_cast<UA_String*> (UA_Array_new(input.size(), &UA_TYPES[UA_TYPES_STRING])) ;
    for (unsigned int i=0; i<input.size(); ++i)
    {
        UaStatus status = UA_String_copy( input[i].impl(), &array[i] );
        if (!status.isGood())
            throw std::runtime_error("UA_String_copy:"+status.toString().toUtf8());
    }
    UA_Variant_setArray( m_impl, array, input.size(), &UA_TYPES[UA_TYPES_STRING]);

}

UaStatus UaVariant::toBool( OpcUa_Boolean& out ) const
{
    return toSimpleType( &UA_TYPES[UA_TYPES_BOOLEAN], &out );
}

UaStatus UaVariant::toInt16( OpcUa_Int16& out ) const
{
    return toSimpleType( &UA_TYPES[UA_TYPES_INT16], &out );
}

UaStatus UaVariant::toUInt16( OpcUa_UInt16& out) const
{
	return toSimpleType( &UA_TYPES[UA_TYPES_UINT16], &out );
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

UaStatus UaVariant::toUInt64( OpcUa_UInt64& out ) const
{
    return toSimpleType( &UA_TYPES[UA_TYPES_UINT64], &out );
}

UaStatus UaVariant::toByte(OpcUa_Byte& out) const
{
	return toSimpleType( &UA_TYPES[UA_TYPES_BYTE], &out );
}

UaStatus UaVariant::toFloat( OpcUa_Float& out ) const
{
    return toSimpleType( &UA_TYPES[UA_TYPES_FLOAT], &out );
}

UaStatus UaVariant::toDouble( OpcUa_Double& out ) const
{
    return toSimpleType( &UA_TYPES[UA_TYPES_DOUBLE], &out );
}

UaStatus UaVariant::toByteString( UaByteString& out) const
{
	if (m_impl->type != &UA_TYPES[UA_TYPES_BYTESTRING])
		throw std::runtime_error("not-a-bytestring-and-conversion-not-implemented");

	UA_ByteString * encapsulated = static_cast<UA_ByteString*> (m_impl->data); // nasty, isn't it?

	out.setByteString( encapsulated->length, encapsulated->data );

	return OpcUa_Good;
}

UaString UaVariant::toString( ) const
{
    if (m_impl->type != &UA_TYPES[UA_TYPES_STRING])
        throw std::runtime_error("not-a-string");
    return UaString( (UA_String*)m_impl->data );
}

UaString UaVariant::toFullString() const
{
	std::ostringstream result;
	if(m_impl)
	{
		if(m_impl->type == &UA_TYPES[UA_TYPES_STRING] && isScalarValue())
		{
			result << toString().toUtf8();
		}
		else
		{
			result << "type ["<<(m_impl->type)<<"] dimensions count ["<<m_impl->arrayDimensionsSize<<"]";
		}
	}
	else
	{
		result << "type [EMPTY!]";
	}
	return UaString(result.str().c_str());
}



template<typename T>
UaStatus UaVariant::toSimpleType( const UA_DataType* targetDataType, T* out ) const
{

    if (!m_impl || !m_impl->data)
    {
    	LOG(Log::DBG) << __FUNCTION__ << " conversion failed, variant is null or uninitialized";
        return OpcUa_Bad;
    }

    if(!m_impl->type || !targetDataType)
    {
    	LOG(Log::DBG) << __FUNCTION__ << " conversion failed, variant data type ["<<(m_impl->type?m_impl->type->typeName:"NULL!")<<"] target data type ["<<(targetDataType?targetDataType->typeName:"NULL!")<<"]";
    	return OpcUa_Bad;
    }

    // handle simplest case: no conversion
    if(m_impl->type  == targetDataType)
    {
    	*out = *static_cast<T*>(m_impl->data);
    	return OpcUa_Good;
    }

    // handle numeric conversion
    if(isNumericType(*m_impl->type) && isNumericType(*targetDataType))
    {
    	return convertNumericType(out);
    }

    // no conversion possible - failure
	LOG(Log::DBG) << __FUNCTION__ << " conversion failed; cannot convert between variant data type ["<<m_impl->type->typeName<<"] and target data type ["<<targetDataType->typeName<<"]";
	return OpcUa_Bad;
}

template<typename TTargetNumericType>
  UaStatus UaVariant::convertNumericType(TTargetNumericType* out ) const
{
	try
	{
		switch(m_impl->type->typeIndex)
		{
			case UA_TYPES_BOOLEAN:
				*out = boost::numeric_cast<TTargetNumericType>( *(static_cast<OpcUa_Boolean*>(m_impl->data)) );
				return OpcUa_Good;
			case UA_TYPES_BYTE:
				*out = boost::numeric_cast<TTargetNumericType>( *(static_cast<OpcUa_Byte*>(m_impl->data)) );
				return OpcUa_Good;
			case UA_TYPES_SBYTE:
				*out = boost::numeric_cast<TTargetNumericType>( *(static_cast<OpcUa_SByte*>(m_impl->data)) );
				return OpcUa_Good;
			case UA_TYPES_INT16:
				*out = boost::numeric_cast<TTargetNumericType>( *(static_cast<OpcUa_Int16*>(m_impl->data)) );
				return OpcUa_Good;
			case UA_TYPES_UINT16:
				*out = boost::numeric_cast<TTargetNumericType>( *(static_cast<OpcUa_UInt16*>(m_impl->data)) );
				return OpcUa_Good;
			case UA_TYPES_INT32:
				*out = boost::numeric_cast<TTargetNumericType>( *(static_cast<OpcUa_Int32*>(m_impl->data)) );
				return OpcUa_Good;
			case UA_TYPES_UINT32:
				*out = boost::numeric_cast<TTargetNumericType>( *(static_cast<OpcUa_UInt32*>(m_impl->data)) );
				return OpcUa_Good;
			case UA_TYPES_INT64:
				*out = boost::numeric_cast<TTargetNumericType>( *(static_cast<OpcUa_Int64*>(m_impl->data)) );
				return OpcUa_Good;
			case UA_TYPES_UINT64:
				*out = boost::numeric_cast<TTargetNumericType>( *(static_cast<OpcUa_UInt64*>(m_impl->data)) );
				return OpcUa_Good;
			case UA_TYPES_FLOAT:
				*out = boost::numeric_cast<TTargetNumericType>( *(static_cast<OpcUa_Float*>(m_impl->data)) );
				return OpcUa_Good;
			case UA_TYPES_DOUBLE:
				*out = boost::numeric_cast<TTargetNumericType>( *(static_cast<OpcUa_Double*>(m_impl->data)) );
				return OpcUa_Good;
			default:
				LOG(Log::DBG) << __FUNCTION__ << " conversion failed, no conversion handler for internal variant type ["<<m_impl->type->typeName<<"]";
				return OpcUa_Bad;
		}
	}
	catch(const boost::numeric::bad_numeric_cast& e)
	{
		LOG(Log::DBG) << __FUNCTION__ << " conversion failed for internal variant type ["<<m_impl->type->typeName<<"], boost conversion threw exception: " << e.what();
		return OpcUa_BadOutOfRange;
	}
	catch(const std::runtime_error& e)
	{
		LOG(Log::DBG) << __FUNCTION__ << " conversion failed for internal variant type ["<<m_impl->type->typeName<<"], conversion threw exception: " << e.what();
		return OpcUa_Bad;
	}
}

bool UaVariant::isNumericType( const UA_DataType& dataType ) const
{
	switch(dataType.typeIndex)
	{
	case UA_TYPES_BOOLEAN:
	case UA_TYPES_SBYTE:
	case UA_TYPES_BYTE:
	case UA_TYPES_INT16:
	case UA_TYPES_UINT16:
	case UA_TYPES_INT32:
	case UA_TYPES_UINT32:
	case UA_TYPES_INT64:
	case UA_TYPES_UINT64:
	case UA_TYPES_FLOAT:
	case UA_TYPES_DOUBLE:
		return true;
	default:
		return false;
	}
}

template<typename T, typename U>
OpcUa_StatusCode UaVariant::toArray( const UA_DataType* dataType, U& out) const
{
    if (UA_Variant_hasArrayType(m_impl, dataType ))
    {
        size_t sz = m_impl->arrayLength;
        out.create( sz );
        T* input = static_cast<T*> (m_impl->data);
        std::copy(input, input+sz, out.begin() );
        return OpcUa_Good;
    }
    else
        return OpcUa_BadDataEncodingInvalid;
}

OpcUa_StatusCode UaVariant::toBoolArray( UaBooleanArray& out ) const
{
    return this->toArray<OpcUa_Boolean, UaBooleanArray>( &UA_TYPES[UA_TYPES_BOOLEAN], out );
}

OpcUa_StatusCode UaVariant::toSByteArray( UaSByteArray& out ) const
{
    return this->toArray<OpcUa_SByte, UaSByteArray>( &UA_TYPES[UA_TYPES_SBYTE], out );
}

OpcUa_StatusCode UaVariant::toByteArray( UaByteArray& out ) const
{
    return this->toArray<OpcUa_Byte, UaByteArray>( &UA_TYPES[UA_TYPES_BYTE], out );
}

OpcUa_StatusCode UaVariant::toInt16Array( UaInt16Array& out ) const
{
    return this->toArray<OpcUa_Int16, UaInt16Array>( &UA_TYPES[UA_TYPES_INT16], out );
}

OpcUa_StatusCode UaVariant::toUInt16Array( UaUInt16Array& out ) const
{
    return this->toArray<OpcUa_UInt16, UaUInt16Array>( &UA_TYPES[UA_TYPES_UINT16], out );
}

OpcUa_StatusCode UaVariant::toInt32Array( UaInt32Array& out ) const
{
    return this->toArray<OpcUa_Int32, UaInt32Array>( &UA_TYPES[UA_TYPES_INT32], out );
}

OpcUa_StatusCode UaVariant::toUInt32Array( UaUInt32Array& out ) const
{
    return this->toArray<OpcUa_UInt32, UaUInt32Array>( &UA_TYPES[UA_TYPES_UINT32], out );
}

OpcUa_StatusCode UaVariant::toInt64Array( UaInt64Array& out ) const
{
    return this->toArray<OpcUa_Int64, UaInt64Array>( &UA_TYPES[UA_TYPES_INT64], out );
}

OpcUa_StatusCode UaVariant::toUInt64Array( UaUInt64Array& out ) const
{
    return this->toArray<OpcUa_UInt64, UaUInt64Array>( &UA_TYPES[UA_TYPES_UINT64], out );
}

OpcUa_StatusCode UaVariant::toFloatArray( UaFloatArray& out ) const
{
    return this->toArray<OpcUa_Float, UaFloatArray>( &UA_TYPES[UA_TYPES_FLOAT], out );
}

OpcUa_StatusCode UaVariant::toDoubleArray( UaDoubleArray& out ) const
{
    return this->toArray<OpcUa_Double, UaDoubleArray>( &UA_TYPES[UA_TYPES_DOUBLE], out );
}

OpcUa_StatusCode UaVariant::toStringArray( UaStringArray& out) const
{
    return this->toArray<UA_String, UaStringArray>( &UA_TYPES[UA_TYPES_STRING], out );
}

UaStatus UaVariant::copyTo ( UA_Variant* to) const
{
    return UA_Variant_copy(m_impl, to);
}

/**
    Note(Ben, Piotr):
    arrayDimensions, as of Apr 2018, don't seem fully available in open62541.
    We therefore replaced the previous idea of profiting from them with a simpler approach
    of using only arrayLength.
 */
bool UaVariant::isScalarValue() const
{
	if(m_impl && m_impl->data) // internal value exists and has data
	{
	    return m_impl->arrayLength == 0;
	}
	return false;
}

void UaVariant::arrayDimensions( UaUInt32Array &arrayDimensions ) const
{
    if (isScalarValue())
        arrayDimensions.create(0);
    else
    {
        arrayDimensions.create( 1 ); // handling only 1-dim arrays
        arrayDimensions[0] = m_impl->arrayLength;
    }
}

OpcUa_Boolean UaVariant::isArray ()   const
{
    return !this->isScalarValue();
}
