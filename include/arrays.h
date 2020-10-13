/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
 *
 *  Created on: 1 May, 2017
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

#ifndef OPEN62541_COMPAT_INCLUDE_ARRAYS_H_
#define OPEN62541_COMPAT_INCLUDE_ARRAYS_H_

#include <array_templates.h>
#include <statuscode.h>
#include <stack_fake_structures.h>
#include <other.h>
#include <simple_arrays.h>



typedef UaCompatArray<ReadValueId> UaReadValueIds;
typedef UaCompatArray<DataValue> UaDataValues;
typedef UaCompatArray<DiagnosticInfo> UaDiagnosticInfos;
typedef UaCompatArray<UaStatus> UaStatusCodes;
typedef UaCompatArray<WriteValue> UaWriteValues;
typedef UaCompatArray<OpcUa_StatusCode> UaStatusCodeArray;
typedef UaCompatArray<UaVariant> UaVariantArray;
typedef UaCompatArray<ReferenceDescription> UaReferenceDescriptions;

#endif /* OPEN62541_COMPAT_INCLUDE_ARRAYS_H_ */
