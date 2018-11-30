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
#include <other.h>
#include <simple_arrays.h>

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
  UaVariant( const UaByteString& v );
  

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

  void setBoolArray( UaBooleanArray& val, OpcUa_Boolean bDetach = OpcUa_False);
  void setSByteArray( UaSByteArray& val, OpcUa_Boolean bDetach = OpcUa_False);
  void setByteArray( UaByteArray& val, OpcUa_Boolean bDetach = OpcUa_False );
  void setInt16Array( UaInt16Array& val, OpcUa_Boolean bDetach = OpcUa_False);
  void setUInt16Array( UaUInt16Array& val, OpcUa_Boolean bDetach = OpcUa_False );
  void setInt32Array( UaInt32Array& val, OpcUa_Boolean bDetach = OpcUa_False);
  void setUInt32Array( UaUInt32Array& val, OpcUa_Boolean bDetach = OpcUa_False );
  void setInt64Array( UaInt64Array& val, OpcUa_Boolean bDetach = OpcUa_False);
  void setUInt64Array( UaUInt64Array& val, OpcUa_Boolean bDetach = OpcUa_False );
  void setFloatArray( UaFloatArray& val, OpcUa_Boolean bDetach = OpcUa_False);
  void setDoubleArray( UaDoubleArray& val, OpcUa_Boolean bDetach = OpcUa_False );
  void setStringArray( UaStringArray& val, OpcUa_Boolean bDetach = OpcUa_False );

  void clear () {}; // TODO:
  


  // getters
  UaStatus toBool( OpcUa_Boolean& value) const;
  UaStatus toInt16( OpcUa_Int16& value) const;
  UaStatus toUInt16( OpcUa_UInt16& value) const;
  UaStatus toInt32( OpcUa_Int32& value ) const;
  UaStatus toUInt32( OpcUa_UInt32& value ) const;
  UaStatus toInt64( OpcUa_Int64& value ) const;
  UaStatus toByte(OpcUa_Byte& value ) const;
  UaStatus toSByte(OpcUa_SByte& value ) const;
  UaStatus toUInt64( OpcUa_UInt64& value ) const;
  UaStatus toFloat( OpcUa_Float&  value ) const;
  UaStatus toDouble( OpcUa_Double& value ) const;
  UaStatus toByteString( UaByteString& value) const;

  UaString toString( ) const;
  UaString toFullString() const;

  OpcUa_StatusCode toBoolArray( UaBooleanArray& out ) const;
  OpcUa_StatusCode toSByteArray( UaSByteArray& out ) const;
  OpcUa_StatusCode toByteArray( UaByteArray& out ) const;
  OpcUa_StatusCode toInt16Array( UaInt16Array& out ) const;
  OpcUa_StatusCode toUInt16Array( UaUInt16Array& out ) const;
  OpcUa_StatusCode toInt32Array( UaInt32Array& out ) const;
  OpcUa_StatusCode toUInt32Array( UaUInt32Array& out ) const;
  OpcUa_StatusCode toInt64Array( UaInt64Array& out ) const;
  OpcUa_StatusCode toUInt64Array( UaUInt64Array& out ) const;
  OpcUa_StatusCode toFloatArray( UaFloatArray& out ) const;
  OpcUa_StatusCode toDoubleArray( UaDoubleArray& out ) const;
  OpcUa_StatusCode toStringArray( UaStringArray& out) const;

  // copy-To has a signature with UaVariant however it should be the stack type. This is best effort compat we can get at the moment. (pnikiel)
  UaStatus copyTo ( UaVariant* to ) const { *to = UaVariant( *m_impl ); return OpcUa_Good; }
  UaStatus copyTo ( UA_Variant* to) const;

  const UA_Variant* impl() const { return m_impl; }

  void arrayDimensions( UaUInt32Array &arrayDimensions ) const;
  OpcUa_Boolean isArray  () const;

 private:
  static UA_Variant* createAndCheckOpen62541Variant();
  static void destroyOpen62541Variant(UA_Variant* open62541Variant);

  UA_Variant * m_impl;
  //! Will assign a supplied newValue to the variant's value. If possible (matching old/new types) a realloc is avoided.
  void reuseOrRealloc( const UA_DataType* dataType, void* newValue );

  template<typename ArrayType>
  void set1DArray( const UA_DataType* dataType, const ArrayType& input );

  //! Will convert stored value to a simple type, if possible
  template<typename T>
    UaStatus toSimpleType( const UA_DataType* targetDataType, T* out ) const;

  template<typename TTargetNumericType>
    UaStatus convertNumericType(TTargetNumericType* out ) const;

  bool isNumericType( const UA_DataType& dataType ) const;

  template<typename T, typename U>
  OpcUa_StatusCode toArray( const UA_DataType* dataType, U& out) const;

  bool isScalarValue() const;
};



#endif // __UAVARIANT_H__
