/*
 * uadatavariablecache.cpp
 *
 *  Created on: May 1, 2017
 *      Author: pnikiel
 */


#include <uadatavariablecache.h>
#include <stdexcept>



UaPropertyMethodArgument::UaPropertyMethodArgument  (
			const UaNodeId &nodeId,
			OpcUa_Byte     accessLevel,
			OpcUa_UInt32   numberOfArguments,
			ArgumentType   argumentType):
			m_nodeId(nodeId),
			m_numberArguments( numberOfArguments ),
			m_browseName(0, "args"),
			m_impl( new UA_Argument* [ numberOfArguments] ),
			m_argumentType(argumentType)
{

	for (unsigned int i=0; i<numberOfArguments; ++i)
		m_impl[i] = UA_Argument_new();

}

OpcUa_StatusCode UaPropertyMethodArgument::setArgument 	(
			OpcUa_UInt32  	        index,
			const UaString &  	    name,
			const UaNodeId &  	    dataType,
			OpcUa_Int32  	        valueRank,
			const UaUInt32Array &  	arrayDimensions,
			const UaLocalizedText & description
		)
{
	if (index >= m_numberArguments)
		return OpcUa_BadInvalidArgument; // TODO more refined error code


	m_impl[index]->dataType = dataType.impl();

	if( UA_String_copy(name.impl(), &m_impl[index]->name) != OpcUa_Good )
		return OpcUa_Bad;
	m_impl[index]->valueRank = valueRank;

	return OpcUa_Good;

}

const UA_Argument& UaPropertyMethodArgument::implArgument (unsigned int index) const
{
	if (index >= m_numberArguments)
		throw std::runtime_error("wrong arg");
	return *m_impl[index];
}

UaPropertyCache::UaPropertyCache (
		const UaString  &name,
		const UaNodeId  &nodeId,
		const UaVariant &defaultValue,
		OpcUa_Byte      accessLevel,
		const UaString& defaultLocaleId) :
				m_nodeId(nodeId),
				m_browseName(0, name)
{}
