/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
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


#ifndef __UAVARIANT_H__
#define __UAVARIANT_H__

#include <opcua_platformdefs.h>
#include <open62541.h>

enum OpcUaType
  {

    OpcUaType_Null,
    OpcUaType_Boolean,

    OpcUaType_Byte,
    OpcUaType_SByte,

    OpcUaType_UInt16,
    OpcUaType_Int16,

    OpcUaType_UInt32,
    OpcUaType_Int32,
    OpcUaType_UInt64,
    OpcUaType_Int64,
    OpcUaType_Float,
    OpcUaType_Double,
    OpcUaType_String
  };



class UaVariant
{
 public:
  UaVariant ();
  UaVariant( const UaVariant& other);
  void operator= (const UaVariant &other);
  UaVariant( const UA_Variant& other );

  UaVariant( const UaString& );
  UaVariant( OpcUa_UInt32 v );
  
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

  void clear () {}; // TODO:
  
  // getters
  UaStatus toBool( OpcUa_Boolean& value) const;
  UaStatus toInt16( OpcUa_Int16& value) const;
  UaStatus toInt32( OpcUa_Int32& value ) const;
  UaStatus toUInt32( OpcUa_UInt32& value ) const;
  UaStatus toInt64( OpcUa_Int64& value ) const;
  UaStatus toUInt64( OpcUa_UInt64& value ) const;
  UaStatus toFloat( OpcUa_Float&  value ) const;
  UaStatus toDouble( OpcUa_Double& value ) const;

  UaString toString( ) const;
  
  const UA_Variant* impl() const { return m_impl; }
 private:

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

  private:
    UA_DataValue *m_impl;
  
  											 
};


#endif // __UAVARIANT_H__
