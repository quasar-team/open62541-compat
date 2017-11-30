/*
 * stack_fake_structures.h
 *
 *  Created on: 27 Nov 2017
 *      Author: pnikiel
 */

#ifndef OPEN62541_COMPAT_INCLUDE_UACLIENT_STACK_FAKE_STRUCTURES_H_
#define OPEN62541_COMPAT_INCLUDE_UACLIENT_STACK_FAKE_STRUCTURES_H_

#include <opcua_attributes.h>
#include <uanodeid.h>
#include <uadatavalue.h>

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

#endif /* OPEN62541_COMPAT_INCLUDE_UACLIENT_STACK_FAKE_STRUCTURES_H_ */
