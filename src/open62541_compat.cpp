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
#include <open62541_compat_common.h>



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



};
