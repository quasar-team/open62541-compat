/* © Copyright Piotr Nikiel, CERN, 2017.  All rights not expressly granted are reserved.
 *  stack_fake_structures.h
 *
 *  Created on: 27 Nov, 2017
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

#ifndef OPEN62541_COMPAT_INCLUDE_UACLIENT_STACK_FAKE_STRUCTURES_H_
#define OPEN62541_COMPAT_INCLUDE_UACLIENT_STACK_FAKE_STRUCTURES_H_

#include <opcua_attributes.h>
#include <opcua_types.h>
#include <uanodeid.h>
#include <uadatavalue.h>
#include <uaexpandednodeid.h>


struct ReadValueId
{
    ReadValueId() : NodeId(0, 0), AttributeId (OpcUa_Attributes_Value) {}
    UaNodeId   NodeId;
    Attributes AttributeId;
};

struct DataValue
{
    UaStatus    StatusCode;
    UaVariant   Value;
    UaDateTime  SourceTimestamp;
    UaDateTime  ServerTimestamp;

};

struct WriteValue
{
    WriteValue(): NodeId(0, 0), AttributeId( OpcUa_Attributes_Value), Value() {}
    UaNodeId      NodeId;
    Attributes    AttributeId;
    DataValue     Value;
};

struct ReferenceDescription
{
	UaNodeId         ReferenceTypeId;
	OpcUa_Boolean    IsForward;
	UaExpandedNodeId NodeId;
	UaString         BrowseName; // TODO: should be rather QualifiedName type, but keeping UaString for simplicity ...
	OpcUa_NodeClass  NodeClass;
	UaExpandedNodeId TypeDefinition;
};

#endif /* OPEN62541_COMPAT_INCLUDE_UACLIENT_STACK_FAKE_STRUCTURES_H_ */
