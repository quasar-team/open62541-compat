/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
 * opcua_basedatavariabletype.cpp
 *
 *  Created on: 12 Jun 2018 (by splitting open62541_compat.cpp)
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

#include <opcua_basedatavariabletype.h>

namespace OpcUa
{

BaseDataVariableType::BaseDataVariableType(
    const UaNodeId&    nodeId,
    const UaString&    name,
    OpcUa_UInt16       browseNameNameSpaceIndex,
    const UaVariant&   initialValue,
    OpcUa_Byte         accessLevel,
    NodeManagerConfig* pNodeConfig,
    UaMutexRefCounted* pSharedMutex):

    m_browseName( browseNameNameSpaceIndex, name),
    m_currentValue( initialValue, OpcUa_Good, UaDateTime::now(), UaDateTime::now() ),
    m_nodeId (nodeId),
    m_typeDefinitionId( OpcUaType_Variant, 0),
    m_valueRank(-1), // by default: scalar
    m_accessLevel(accessLevel),
	m_valueHandling(ValueHandling::UaVariable_Value_CacheIsSource), // the behaviour of this class prior to v1.3.8 of open62541-compat matches this ValueHandling mode.,
	m_associatedServer (nullptr)

{

}

UaStatus BaseDataVariableType::setValue(
    Session *session,
    const UaDataValue& dataValue,
    OpcUa_Boolean checkAccessLevel
)
{
    if (!checkAccessLevel || (m_accessLevel & UA_ACCESSLEVELMASK_WRITE))
    {
    	// okay, at least we know that it's writable.
    	switch (m_valueHandling)
    	{
    		case ValueHandling::UaVariable_Value_None: OPEN62541_COMPAT_LOG_AND_THROW(std::logic_error, "This mode of ValueHandling requires that setValue(...) is overridden in a subclass.");
    		case ValueHandling::UaVariable_Value_Cache:
    		{
    			UaStatus status = requestWriteToServer(m_nodeId.impl(), dataValue);
    			if (status.isGood())
    				m_currentValue = dataValue;
    			return status;
    		}

    		case ValueHandling::UaVariable_Value_CacheIsSource:
    			m_currentValue = dataValue;
    			return OpcUa_Good;

    		default: OPEN62541_COMPAT_LOG_AND_THROW(std::logic_error, "Not-implemented");
    	}
    }
    else
        return OpcUa_BadUserAccessDenied;
}

UaDataValue BaseDataVariableType::value(Session* session)
{
	switch (m_valueHandling)
	{
		case ValueHandling::UaVariable_Value_None: OPEN62541_COMPAT_LOG_AND_THROW(std::logic_error, "This mode of ValueHandling requires that value(...) is overridden in a subclass.");
		case ValueHandling::UaVariable_Value_Cache:
			return requestReadFromServer();

		case ValueHandling::UaVariable_Value_CacheIsSource:
		    return m_currentValue.clone();

		default: OPEN62541_COMPAT_LOG_AND_THROW(std::logic_error, "Not-implemented");
	}
}

UaStatus BaseDataVariableType::requestWriteToServer (UA_NodeId variableNodeId, const UaDataValue& dataValue)
{
	if (!m_associatedServer)
		OPEN62541_COMPAT_LOG_AND_THROW(std::logic_error, "Can't perform write request before knowing which server to talk to. Perhaps wrong ValueHandling mode?");
	return UA_Server_writeValue(m_associatedServer, variableNodeId, *dataValue.value()->impl());
}

UaDataValue BaseDataVariableType::requestReadFromServer ()
{
	// TODO: fake
	UaDataValue dv (UaVariant(), OpcUa_Good, UaDateTime::now(), UaDateTime::now());
	return dv;
}

void BaseDataVariableType::associateServer( UA_Server* server )
{
	if (m_associatedServer)
		OPEN62541_COMPAT_LOG_AND_THROW(std::logic_error, "Changing already associated server is probably a logic error?");
	m_associatedServer = server;
}

}

