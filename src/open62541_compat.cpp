/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
 * open62541_compat.cpp
 *
 *  Created on: 15 Nov,  2015
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

#include <open62541_compat.h>
#include <iostream>
#include <sstream>
#include <bitset>
#include <open62541_compat_common.h>

bool UaStatus::isBad() const
{
	return std::bitset<32>(m_status).test(31); // 31 ? BAD defines start at 0x8000000 (OPC-UA specification).
}

UaQualifiedName::UaQualifiedName(int ns, const UaString& name):
    m_unqualifiedName( name )
{
    m_impl.name = *m_unqualifiedName.impl();
    m_impl.namespaceIndex = ns;
}

UaLocalizedText::UaLocalizedText( const char* locale, const char* text) 
{
    m_impl = UA_LOCALIZEDTEXT_ALLOC( locale, text ); 
}

UaLocalizedText::~UaLocalizedText ()
{
    UA_LocalizedText_deleteMembers( &m_impl );
}

UaNode::UaNode ()
{
}

namespace OpcUa
{

    BaseObjectType::BaseObjectType(
        const UaNodeId& nodeId,
        const UaString& name,
        OpcUa_UInt16  browseNameNameSpaceIndex, 
        NodeManagerConfig *nm
        ):
        m_nodeId(nodeId),
        m_browseName( browseNameNameSpaceIndex, name )
    {
        //      std::cout << __PRETTY_FUNCTION__ << std::endl;
        //      std::cout << "nodeId=" << nodeId.toString().toUtf8() << std::endl;
    }

	BaseMethod::BaseMethod (
			const UaNodeId &nodeId,
			const UaString &name,
			OpcUa_UInt16 browseNameNameSpaceIndex,
			UaMutexRefCounted *pSharedMutex):
				m_nodeId(nodeId),
				m_browseName(browseNameNameSpaceIndex, name)

	{

	}


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
		m_nodeId (nodeId)
							   
    {
        //      std::cout << __PRETTY_FUNCTION__ <<" (nodeId="<<nodeId.toString().toUtf8()<<")" << std::endl;
    }


    UaStatus BaseDataVariableType::setValue( 
        Session *session,
        const UaDataValue& dataValue,
        OpcUa_Boolean checkAccessLevel
        )
    {
        // TODO check write mask
        // TODO
        m_currentValue = dataValue;
        return OpcUa_Good;
    }

    UaDataValue BaseDataVariableType::value(Session* session) 
    {
        return m_currentValue.clone();
    }
};
