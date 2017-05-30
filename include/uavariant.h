/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
 *
 *  Created on: 15 Nov,  2015
 *      Author: Piotr Nikiel <piotr@nikiel.info>
 *      Author: bfarnham
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


#ifndef __UAVARIANT_H__
#define __UAVARIANT_H__

#include <opcua_platformdefs.h>
#include <open62541.h>
#include <statuscode.h>
#include <uadatetime.h>
#include <uabytestring.h>

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

enum OpcUaType
  {

    OpcUaType_Null       =  0,
    OpcUaType_Boolean    =  UA_NS0ID_BOOLEAN,
    OpcUaType_SByte      =  UA_NS0ID_SBYTE,    
    OpcUaType_Byte       =  UA_NS0ID_BYTE,
    OpcUaType_Int16      =  UA_NS0ID_INT16, 
    OpcUaType_UInt16     =  UA_NS0ID_UINT16, 
    OpcUaType_Int32      =  UA_NS0ID_INT32,
    OpcUaType_UInt32     =  UA_NS0ID_UINT32,
    OpcUaType_Int64      =  UA_NS0ID_INT64,
    OpcUaType_UInt64     =  UA_NS0ID_UINT64,
    OpcUaType_Float      =  UA_NS0ID_FLOAT,
    OpcUaType_Double     =  UA_NS0ID_DOUBLE,
    OpcUaType_String     =  UA_NS0ID_STRING,
    // support for datetime and guid missing in the compat module
    OpcUaType_ByteString =  UA_NS0ID_BYTESTRING,
	OpcUaType_Variant    =  UA_NS0ID_BASEDATATYPE
    // support for remaining types, i.e. nodeid or statuscode also still missing
  };



class UaVariant
{
 public:
  UaVariant ();
  UaVariant( const UaVariant& other);
  void operator= (const UaVariant &other);
  bool operator==(const UaVariant&) const;
  UaVariant( const UA_Variant& other );

  UaVariant( const UaString& v );
  UaVariant( OpcUa_UInt32 v );
  UaVariant( OpcUa_Int32 v );
  UaVariant( OpcUa_UInt64 v );
  UaVariant( OpcUa_Int64 v );
  UaVariant( OpcUa_Float v );
  UaVariant( OpcUa_Double v );
  UaVariant( OpcUa_Boolean v );
  

  ~UaVariant();
  OpcUaType type() const;
    
  // setters
  void setBool( OpcUa_Boolean value );
  void setByte( OpcUa_Byte value );
  void setSByte( OpcUa_SByte value );
  void setInt16( OpcUa_Int16 value );
  void setUInt16( OpcUa_UInt16 value );
  void setInt32( OpcUa_Int32 value );
  void setUInt32( OpcUa_UInt32 value );
  void setInt64( OpcUa_Int64 value );
  void setUInt64( OpcUa_UInt64 value );

  void setFloat( OpcUa_Float value );
  void setDouble( OpcUa_Double value );

  void setString( const UaString& value );

  void setByteString( const UaByteString& value, bool detach);

  void clear () {}; // TODO:
  
  // getters
  UaStatus toBool( OpcUa_Boolean& value) const;
  UaStatus toInt16( OpcUa_Int16& value) const;
  UaStatus toUInt16( OpcUa_UInt16& value) const;
  UaStatus toInt32( OpcUa_Int32& value ) const;
  UaStatus toUInt32( OpcUa_UInt32& value ) const;
  UaStatus toInt64( OpcUa_Int64& value ) const;
  UaStatus toByte(OpcUa_Byte& value ) const;
  UaStatus toUInt64( OpcUa_UInt64& value ) const;
  UaStatus toFloat( OpcUa_Float&  value ) const;
  UaStatus toDouble( OpcUa_Double& value ) const;
  UaStatus toByteString( UaByteString& value) const;

  UaString toString( ) const;
  UaString toFullString() const;
  
  // copy-To has a signature with UaVariant however it should be the stack type. This is best effort compat we can get at the moment. (pnikiel)
  UaStatus copyTo ( UaVariant* to ) { *to = UaVariant( *m_impl ); return OpcUa_Good; }

  const UA_Variant* impl() const { return m_impl; }
 private:
  static UA_Variant* createAndCheckOpen62541Variant();
  static void destroyOpen62541Variant(UA_Variant* open62541Variant);

  UA_Variant * m_impl;
  //! Will assign a supplied newValue to the variant's value. If possible (matching old/new types) a realloc is avoided.
  void reuseOrRealloc( const UA_DataType* dataType, void* newValue );
  //! Will convert stored value to a simple type, if possible
  template<typename T>
    UaStatus toSimpleType( const UA_DataType* dataType, T* out ) const;
};


class UaDataValue 
{
  public:
    UaDataValue( const UaVariant& variant, OpcUa_StatusCode statusCode, const UaDateTime& serverTime, const UaDateTime& sourceTime );
    UaDataValue( const UaDataValue& other );
    void operator=(const UaDataValue& other );

    ~UaDataValue ();

    const UA_DataValue* impl() const { return m_impl; }
    UaVariant* value() const{ return new UaVariant(m_impl->value); }

    UaDataValue clone(); // can't be const because of synchronization

  private:
    UA_DataValue *m_impl;
    std::atomic_flag m_lock;
    
  
  											 
};


#endif // __UAVARIANT_H__
