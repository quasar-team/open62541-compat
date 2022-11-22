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

#include <uanodeid.h>
#include <uanode.h>
#include <uadatavalue.h>
#include <nodemanagerbase.h>
#include <open62541_compat.h>
#include <statuscode.h>

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
    virtual OpcUa_Byte accessLevel() const { return m_accessLevel; }
    virtual UaDataValue value(Session* session) ;
    virtual UaQualifiedName browseName() const override { return m_browseName; }
    virtual void setAccessLevel (OpcUa_Byte accessLevel) { m_accessLevel = accessLevel; }
    virtual UaNodeId typeDefinitionId() const override { return m_typeDefinitionId; }
    virtual void setDataType( const UaNodeId& typeref ) { m_typeDefinitionId = typeref; }
    virtual void setValueRank (OpcUa_Int32 valueRank) { m_valueRank = valueRank; }
    virtual void setArrayDimensions( const UaUInt32Array &arrayDimensions ) {m_arrayDimensions = arrayDimensions;}
    virtual OpcUa_NodeClass nodeClass() const override { return OpcUa_NodeClass_Variable; }
    virtual UaNodeId nodeId() const override { return m_nodeId; }
    virtual OpcUa_Int32 valueRank() const { return m_valueRank; }
    virtual void arrayDimensions(UaUInt32Array &    arrayDimensions )   const { arrayDimensions = m_arrayDimensions;}
    const UA_DataValue* valueImpl() const { return m_currentValue.impl(); }

private:
    UaQualifiedName m_browseName;
    UaDataValue m_currentValue;
    UaNodeId m_nodeId;
    UaNodeId m_typeDefinitionId;
    OpcUa_Int32 m_valueRank;
    OpcUa_Byte m_accessLevel;
    UaUInt32Array m_arrayDimensions;
};



}

#endif // __OPCUA_BASEDATAVARIABLETYPE_H__
