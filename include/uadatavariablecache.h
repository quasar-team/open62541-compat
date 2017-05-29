/*
 * uadatavariablecache.h
 *
 *  Created on: May 1, 2017
 *      Author: pnikiel
 */

#ifndef OPEN62541_COMPAT_INCLUDE_UADATAVARIABLECACHE_H_
#define OPEN62541_COMPAT_INCLUDE_UADATAVARIABLECACHE_H_

#include <uanode.h>
#include <statuscode.h>
#include <uabasenodes.h>

class UaUInt32Array
{

};


class UaPropertyMethodArgument: public UaNode
{


public:

	enum ArgumentType
	{
		INARGUMENTS,
		OUTARGUMENTS
	};

	UaPropertyMethodArgument (
			const UaNodeId &nodeId,
			OpcUa_Byte     accessLevel,
			OpcUa_UInt32   numberOfArguments,
			ArgumentType   argumentType);

	virtual UaNodeId nodeId() const { return m_nodeId; }
	virtual UaNodeId typeDefinitionId() const { return UaNodeId(UA_NS0ID_BASEDATAVARIABLETYPE,0); }
	virtual OpcUa_NodeClass nodeClass() const { return OpcUa_NodeClass_Variable; }
	virtual UaQualifiedName browseName() const { return m_browseName; }

	OpcUa_StatusCode setArgument 	(
			OpcUa_UInt32  	        index,
			const UaString &  	    name,
			const UaNodeId &  	    dataType,
			OpcUa_Int32  	        valueRank,
			const UaUInt32Array &  	arrayDimensions,
			const UaLocalizedText & description
		) 	;

	// returns UA_Argument per given argument in this property
	const UA_Argument& implArgument (unsigned int index) const;

	unsigned int numArguments () const { return m_numberArguments; }

	ArgumentType argumentType () const { return m_argumentType; }

private:
	const UaNodeId        m_nodeId;
	const unsigned int    m_numberArguments;
	const UaQualifiedName m_browseName;
	UA_Argument **        m_impl;
	const ArgumentType    m_argumentType;

};



#endif /* OPEN62541_COMPAT_INCLUDE_UADATAVARIABLECACHE_H_ */
