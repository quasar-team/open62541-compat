/* © Copyright Piotr Nikiel, CERN, 2017.  All rights not expressly granted are reserved.
 *  opcua_attributes.h
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


#ifndef OPEN62541_COMPAT_INCLUDE_OPCUA_ATTRIBUTES_H_
#define OPEN62541_COMPAT_INCLUDE_OPCUA_ATTRIBUTES_H_


enum Attributes
{
    OpcUa_Attributes_NodeId = UA_ATTRIBUTEID_NODEID,
    OpcUa_Attributes_NodeClass = UA_ATTRIBUTEID_NODECLASS,
    OpcUa_Attributes_BrowseName = UA_ATTRIBUTEID_BROWSENAME,
    OpcUa_Attributes_DisplayName = UA_ATTRIBUTEID_DISPLAYNAME,
    OpcUa_Attributes_Description = UA_ATTRIBUTEID_DESCRIPTION,
    OpcUa_Attributes_WriteMask = UA_ATTRIBUTEID_WRITEMASK,
    OpcUa_Attributes_UserWriteMask = UA_ATTRIBUTEID_USERWRITEMASK,
    OpcUa_Attributes_IsAbstract = UA_ATTRIBUTEID_ISABSTRACT,
    OpcUa_Attributes_Symmetric = UA_ATTRIBUTEID_SYMMETRIC,
    OpcUa_Attributes_InverseName = UA_ATTRIBUTEID_INVERSENAME,
    OpcUa_Attributes_ContainsNoLoops = UA_ATTRIBUTEID_CONTAINSNOLOOPS,
    OpcUa_Attributes_EventNotifier = UA_ATTRIBUTEID_EVENTNOTIFIER,
    OpcUa_Attributes_Value = UA_ATTRIBUTEID_VALUE,
    OpcUa_Attributes_DataType = UA_ATTRIBUTEID_DATATYPE,
    OpcUa_Attributes_ValueRank = UA_ATTRIBUTEID_VALUERANK,
    OpcUa_Attributes_ArrayDimensions = UA_ATTRIBUTEID_ARRAYDIMENSIONS,
    OpcUa_Attributes_AccessLevel = UA_ATTRIBUTEID_ACCESSLEVEL,
    OpcUa_Attributes_UserAccessLevel = UA_ATTRIBUTEID_USERACCESSLEVEL,
    OpcUa_Attributes_MinimumSamplingInterval = UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL,
    OpcUa_Attributes_Historizing = UA_ATTRIBUTEID_HISTORIZING,
    OpcUa_Attributes_Executable = UA_ATTRIBUTEID_EXECUTABLE,
    OpcUa_Attributes_UserExecutable = UA_ATTRIBUTEID_USEREXECUTABLE,
    OpcUa_Attributes_DataTypeDefinition = UA_ATTRIBUTEID_DATATYPEDEFINITION
	/* the following are not yet in open62541, at least not in 1.1.6. TODO!
	OpcUa_Attributes_RolePermissions
	OpcUa_Attributes_UserRolePermissions
	OpcUa_Attributes_AccessRestrictions
	OpcUa_Attributes_AccessLevelEx
	*/

};


#endif /* OPEN62541_COMPAT_INCLUDE_OPCUA_ATTRIBUTES_H_ */
