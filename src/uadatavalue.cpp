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

#include <uadatavalue.cpp>

UaDataValue::UaDataValue( const UaVariant& variant, OpcUa_StatusCode statusCode, const UaDateTime& serverTime, const UaDateTime& sourceTime ):
m_lock()
{
    m_lock.clear();
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


