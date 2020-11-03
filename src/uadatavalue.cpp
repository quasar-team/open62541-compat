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

UaDataValue::UaDataValue( const UaVariant& variant, OpcUa_StatusCode statusCode, const UaDateTime& serverTime, const UaDateTime& sourceTime ):
m_lock()
{
    m_lock.clear();
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

UaDataValue::UaDataValue( const UaDataValue& other ):
            m_lock()
{
    m_lock.clear();
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


