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

typedef UaCompatArray<OpcUa_Int16>  UaInt16Array;
typedef UaCompatArray<OpcUa_UInt16> UaUInt16Array;
typedef UaCompatArray<OpcUa_Int32>  UaInt32Array;
typedef UaCompatArray<OpcUa_UInt32> UaUInt32Array;
typedef UaCompatArray<OpcUa_Float>  UaFloatArray;
typedef UaCompatArray<OpcUa_Double> UaDoubleArray;

#endif /* OPEN62541_COMPAT_INCLUDE_SIMPLE_ARRAYS_H_ */
