/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
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


#ifndef __OPCUA_BASEOBJECTTYPE_H__
#define __OPCUA_BASEOBJECTTYPE_H__

#include <uanode.h>
#include <nodemanagerbase.h>

namespace OpcUa
{

    class BaseObjectType: public UaNode
    {
    public:
	BaseObjectType(
	    const UaNodeId& nodeId,
	    const UaString& name,
	    OpcUa_UInt16  browseNameNameSpaceIndex, 
	    NodeManagerConfig *nm


	    );

	virtual UaQualifiedName browseName() const { return m_browseName; }
	virtual OpcUa_NodeClass nodeClass() const { return OpcUa_NodeClass_Object; }
	virtual UaNodeId typeDefinitionId() const { return UaNodeId(UA_NS0ID_BASEOBJECTTYPE,0); }
    private:
	UaQualifiedName m_browseName;
    };

}

#endif // __OPCUA_BASEOBJECTTYPE_H__
