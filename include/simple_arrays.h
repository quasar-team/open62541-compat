/* © Copyright Piotr Nikiel, CERN, 2018.  All rights not expressly granted are reserved.
 *  simple_arrays.h
 *
 *  Created on: 12 Mar 2018
 *      Author: Piotr Nikiel <piotr@nikiel.info>
 *
 *       Simple arrays are those which don't require UaVariant declaration,
 *       therefore they can be directly included in UaVariant.h
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

#ifndef OPEN62541_COMPAT_INCLUDE_SIMPLE_ARRAYS_H_
#define OPEN62541_COMPAT_INCLUDE_SIMPLE_ARRAYS_H_

#include <array_templates.h>

/* Piotr:
 * Here is a comment why we have the class below (AvoidStdVectorBoolSpecializationProblem).
 * It comes from a questionable decision of STL that std::vector<bool> is specialized
 * as a bitmap. While it clearly has some advantages (e.g. it is memory compact) it has
 * disadvantages: you can't get a reference to a particular element of the vector.
 * So we have this class below which hopefully evaluates to sizeof(bool) in the terms of
 * memory consumption. First and foremost though, std::vector<> of it is quite sane!
 */

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
