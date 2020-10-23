/* © Copyright Piotr Nikiel, CERN, 2017.  All rights not expressly granted are reserved.
 *  opcua_types.h
 *
 *  Created on: 30 Nov, 2017
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

#ifndef OPEN62541_COMPAT_INCLUDE_OPCUA_TYPES_H_
#define OPEN62541_COMPAT_INCLUDE_OPCUA_TYPES_H_

#include <open62541.h>

enum OpcUa_TimestampsToReturn
{
    OpcUa_TimestampsToReturn_Both = UA_TIMESTAMPSTORETURN_BOTH
};


enum OpcUa_NodeClass
{
    OpcUa_NodeClass_Object = UA_NodeClass::UA_NODECLASS_OBJECT,
    OpcUa_NodeClass_Variable = UA_NodeClass::UA_NODECLASS_VARIABLE,
	OpcUa_NodeClass_Method = UA_NodeClass::UA_NODECLASS_METHOD
};

OpcUa_NodeClass safeConvertNodeClassToSdk (UA_NodeClass nc);

#endif /* OPEN62541_COMPAT_INCLUDE_OPCUA_TYPES_H_ */
