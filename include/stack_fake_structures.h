/*
 * stack_fake_structures.h
 *
 *  Created on: 27 Nov 2017
 *      Author: pnikiel
 */

#ifndef OPEN62541_COMPAT_INCLUDE_UACLIENT_STACK_FAKE_STRUCTURES_H_
#define OPEN62541_COMPAT_INCLUDE_UACLIENT_STACK_FAKE_STRUCTURES_H_

// TODO: Attributes should be moved completely out of this.

#include <uanodeid.h>

enum Attributes
{
    OpcUa_Attributes_Value = UA_ATTRIBUTEID_VALUE
};

struct ReadValueId
{
    ReadValueId() : NodeId(0, 0), AttributeId (OpcUa_Attributes_Value) {}
    UaNodeId   NodeId;
    Attributes AttributeId;
};

struct DataValue
{
    UaStatus   StatusCode;
    UaVariant  Value;

};



#endif /* OPEN62541_COMPAT_INCLUDE_UACLIENT_STACK_FAKE_STRUCTURES_H_ */
