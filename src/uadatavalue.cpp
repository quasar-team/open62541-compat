/*
 * uadatavalue.cpp
 *
 *  Created on: 27 Nov 2017
 *      Author: pnikiel
 *
 *  This code used to live in open62541_compat.cpp since the beginning.
 *  Now moved to this separate file for easier maintenance and separation
 *  of concerns.
 */

#include <stdexcept>

#include <LogIt.h>
#include <uadatavalue.h>
#include <open62541_compat.h>

UaDataValue::UaDataValue( const UaVariant& variant, OpcUa_StatusCode statusCode, const UaDateTime& sourceTime, const UaDateTime& serverTime )
{
    m_impl = UA_DataValue_new ();
    if (!m_impl)
    	OPEN62541_COMPAT_LOG_AND_THROW(
    			std::runtime_error,
				"UA_DataValue_new returned null ptr" );

    // TODO: duplicate the variant
    UA_Variant_copy( variant.impl(), &m_impl->value );
    LOG(Log::TRC) << "After UA_Variant_copy: src="<<variant.impl()<<" src.data="<<variant.impl()->data<<" dst="<<&m_impl->value<<" dst.data="<<m_impl->value.data;

    m_impl->status = statusCode;
    m_impl->hasStatus = 1;

    m_impl->serverTimestamp = static_cast<UA_DateTime>(serverTime);
    m_impl->hasServerTimestamp = UA_TRUE;
    m_impl->hasServerPicoseconds = UA_FALSE;

    m_impl->sourceTimestamp = static_cast<UA_DateTime>(sourceTime);
    m_impl->hasSourceTimestamp = UA_TRUE;
    m_impl->hasSourcePicoseconds = UA_FALSE;

    m_impl->hasValue = 1;

}

UaDataValue::UaDataValue()
{
    m_impl = UA_DataValue_new ();
    if (!m_impl)
    	OPEN62541_COMPAT_LOG_AND_THROW(
    			std::runtime_error,
				"UA_DataValue_new returned null ptr" );
}

void UaDataValue::setValue( UaVariant& value, OpcUa_Boolean detachValue, OpcUa_Boolean updateTimeStamps )
{
    std::lock_guard<std::mutex> lock(m_lock);
    UA_Variant_clear( &m_impl->value );
    UA_Variant_copy( value.impl(), &m_impl->value );
    m_impl->hasValue = 1;
    if (detachValue != OpcUa_False)
        value.clear();
    if (updateTimeStamps != OpcUa_False)
    {
        UA_DateTime now = UA_DateTime_now();
        m_impl->sourceTimestamp = now;
        m_impl->hasSourceTimestamp = UA_TRUE;
        m_impl->serverTimestamp = now;
        m_impl->hasServerTimestamp = UA_TRUE;
    }
}

void UaDataValue::setDataValue( UaVariant& value, OpcUa_Boolean detachValue, OpcUa_StatusCode statusCode, const UaDateTime& sourceTimestamp, const UaDateTime& serverTimestamp )
{
    std::lock_guard<std::mutex> lock(m_lock);
    UA_Variant_clear( &m_impl->value );
    UA_Variant_copy( value.impl(), &m_impl->value );
    m_impl->hasValue = 1;
    if (detachValue != OpcUa_False)
        value.clear();
    m_impl->status = statusCode;
    m_impl->hasStatus = 1;
    m_impl->sourceTimestamp = static_cast<UA_DateTime>(sourceTimestamp);
    m_impl->hasSourceTimestamp = UA_TRUE;
    m_impl->serverTimestamp = static_cast<UA_DateTime>(serverTimestamp);
    m_impl->hasServerTimestamp = UA_TRUE;
}

void UaDataValue::setStatusCode( OpcUa_StatusCode statusCode )
{
    std::lock_guard<std::mutex> lock(m_lock);
    m_impl->status = statusCode;
    m_impl->hasStatus = 1;
}

void UaDataValue::setSourceTimestamp( const UaDateTime& sourceTimestamp )
{
    std::lock_guard<std::mutex> lock(m_lock);
    m_impl->sourceTimestamp = static_cast<UA_DateTime>(sourceTimestamp);
    m_impl->hasSourceTimestamp = UA_TRUE;
}

void UaDataValue::setServerTimestamp( const UaDateTime& serverTimestamp )
{
    std::lock_guard<std::mutex> lock(m_lock);
    m_impl->serverTimestamp = static_cast<UA_DateTime>(serverTimestamp);
    m_impl->hasServerTimestamp = UA_TRUE;
}

UaDataValue::UaDataValue( const UaDataValue& other )
{
    m_impl = UA_DataValue_new ();
    // LOG(Log::INF) << "allocated new UA_DataValue @ " <<  m_impl;
    UA_DataValue_copy( other.m_impl, m_impl );
}

void UaDataValue:: operator=(const UaDataValue& other )
{
    std::lock_guard<std::mutex> lock(m_lock);
    if (m_impl)
    {
        UA_DataValue_clear (m_impl);
        UA_DataValue_delete( m_impl );
        m_impl = 0;
    }
    m_impl = UA_DataValue_new ();
    // LOG(Log::INF) << "allocated new UA_DataValue @ " <<  m_impl;
    UA_DataValue_copy( other.m_impl, m_impl );

}

bool UaDataValue:: operator==(const UaDataValue &other) const
{
    if(this == &other) return true;
    if(m_impl->status == other.m_impl->status &&
       m_impl->serverTimestamp == other.m_impl->serverTimestamp &&
       m_impl->sourceTimestamp == other.m_impl->sourceTimestamp)
    {
        // compare values - apparently no direct open62541 comparator, so, compare via compat API
        return UaVariant(m_impl->value) == UaVariant(other.m_impl->value);
    }

    return false;
}

bool UaDataValue:: operator!=(const UaDataValue &other) const
{
    return !(*this == other);
}

UaDataValue UaDataValue::clone()
{
    std::lock_guard<std::mutex> lock(m_lock);
    UaDataValue aCopy ( *this );
    return aCopy;

}

UaDataValue:: ~UaDataValue ()
{
    if (m_impl)
    {
        UA_DataValue_clear( m_impl );
        UA_DataValue_delete( m_impl );
        m_impl = 0;
    }

}


