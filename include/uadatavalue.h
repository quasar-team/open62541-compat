/*
 * uadatavalue.h
 *
 *  Created on: 27 Nov 2017
 *      Author: pnikiel
 */

#ifndef INCLUDE_UADATAVALUE_H_
#define INCLUDE_UADATAVALUE_H_

#include <uavariant.h>

#ifdef __linux__

#define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)
#if GCC_VERSION > 40800
#include <atomic>
#else // GCC_VERSION
#include <stdatomic.h>
#endif // GCC_VERSION

#else //  __linux__ not defined, so windows or so...
#include <atomic>
#endif // __linux__

class UaDataValue
{
  public:
    UaDataValue( const UaVariant& variant, OpcUa_StatusCode statusCode, const UaDateTime& serverTime, const UaDateTime& sourceTime );
    UaDataValue( const UaDataValue& other );
    void operator=(const UaDataValue& other );

    ~UaDataValue ();

    const UA_DataValue* impl() const { return m_impl; }
    const UA_Variant* value() const{ return &m_impl->value; }

    OpcUa_StatusCode statusCode() const { return m_impl->status; }

    UaDataValue clone(); // can't be const because of synchronization

  private:
    UA_DataValue *m_impl;
    std::atomic_flag m_lock;



};

#endif /* INCLUDE_UADATAVALUE_H_ */
