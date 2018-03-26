/*
 * simple_arrays.h
 *
 *  Created on: 12 Mar 2018
 *      Author: pnikiel
 *
 * Simple arrays are those which don't require UaVariant declaration,
 * therefore they can be directly included in UaVariant.h
 */

#ifndef OPEN62541_COMPAT_INCLUDE_SIMPLE_ARRAYS_H_
#define OPEN62541_COMPAT_INCLUDE_SIMPLE_ARRAYS_H_

#include <array_templates.h>

// TODO: describe why we're doing this

struct AvoidStdVectorBoolSpecializationProblem
{
    bool booleanValue;
    void operator= (bool in) { booleanValue=in; }
    operator bool() const { return booleanValue; }
};

typedef UaCompatArray<AvoidStdVectorBoolSpecializationProblem>  UaBooleanArray;
typedef UaCompatArray<OpcUa_SByte>   UaSByteArray;
typedef UaCompatArray<OpcUa_Byte>    UaByteArray;
typedef UaCompatArray<OpcUa_Int16>   UaInt16Array;
typedef UaCompatArray<OpcUa_UInt16>  UaUInt16Array;
typedef UaCompatArray<OpcUa_Int32>   UaInt32Array;
typedef UaCompatArray<OpcUa_UInt32>  UaUInt32Array;
typedef UaCompatArray<OpcUa_Int64>   UaInt64Array;
typedef UaCompatArray<OpcUa_UInt64>  UaUInt64Array;
typedef UaCompatArray<OpcUa_Float>   UaFloatArray;
typedef UaCompatArray<OpcUa_Double>  UaDoubleArray;
typedef UaCompatArray<UaString>      UaStringArray;

#endif /* OPEN62541_COMPAT_INCLUDE_SIMPLE_ARRAYS_H_ */
