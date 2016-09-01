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


#ifndef __OPCUA_BASEDATAVARIABLETYPE_H__
#define __OPCUA_BASEDATAVARIABLETYPE_H__

#include <uanode.h>
#include <nodemanagerbase.h>
#include <open62541_compat.h>

namespace OpcUa
{

    class BaseDataVariableType: public UaNode
    {
    public:
	BaseDataVariableType(
	    const UaNodeId&    nodeId,
	    const UaString&    name,
	    OpcUa_UInt16       browseNameNameSpaceIndex,
	    const UaVariant&   initialValue,
	    OpcUa_Byte         accessLevel,
	    NodeManagerConfig* pNodeConfig,
	    UaMutexRefCounted* pSharedMutex = NULL);
	virtual ~BaseDataVariableType() {};
 
	virtual UaStatus setValue( 
	    Session *session,
	    const UaDataValue& dataValue,
	    OpcUa_Boolean checkAccessLevel
	    );
	virtual UaDataValue value(Session* session) ;
	virtual UaQualifiedName browseName() const { return m_browseName; }
	virtual UaNodeId typeDefinitionId() const { return UaNodeId(UA_NS0ID_BASEDATAVARIABLETYPE,0); }
	virtual void setDataType( const UaNodeId& typeref ) {}
	virtual OpcUa_NodeClass nodeClass() const { return OpcUa_NodeClass_Variable; }

        const UA_DataValue* valueImpl() const { return m_currentValue.impl(); }

    private:
	UaQualifiedName m_browseName;
	UaDataValue m_currentValue;
    };


  
}

#endif // __OPCUA_BASEDATAVARIABLETYPE_H__
