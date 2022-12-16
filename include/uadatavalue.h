/*
 * uadatavalue.h
 *
 *  Created on: 27 Nov 2017
 *      Author: pnikiel
 */

#ifndef INCLUDE_UADATAVALUE_H_
#define INCLUDE_UADATAVALUE_H_

#include <mutex>

#include <uavariant.h>

class UaDataValue
{
  public:
    UaDataValue( const UaVariant& variant, OpcUa_StatusCode statusCode, const UaDateTime& serverTime, const UaDateTime& sourceTime );
    UaDataValue( const UaDataValue& other );
    void operator=(const UaDataValue& other );
    bool operator==(const UaDataValue &other) const;
    bool operator!=(const UaDataValue &other) const;

    ~UaDataValue ();

    const UA_DataValue* impl() const { return m_impl; }
    const UA_Variant* value() const{ return &m_impl->value; }

    OpcUa_StatusCode statusCode() const { return m_impl->status; }

    UaDataValue clone(); // can't be const because of synchronization

  private:
    UA_DataValue *m_impl;

    //! The locking done here is to let UaDataValue be concurrently used from multiple threads 
    // (e.g. network stack for monitoring and a quasar server "above" to publish) to make up for the UASDK behaviour.
    std::mutex m_lock;



};

#endif /* INCLUDE_UADATAVALUE_H_ */
