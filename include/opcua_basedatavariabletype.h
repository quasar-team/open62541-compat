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
#include <uabasenodes.h>

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
    virtual UaNodeId typeDefinitionId() const override { return m_typeDefinitionId; }
    virtual void setDataType( const UaNodeId& typeref ) { m_typeDefinitionId = typeref; }
    virtual void setValueRank (OpcUa_Int32 valueRank) { m_valueRank = valueRank; }
    virtual void setArrayDimensions( const UaUInt32Array &arrayDimensions ) {m_arrayDimensions = arrayDimensions;}
    virtual OpcUa_NodeClass nodeClass() const override { return OpcUa_NodeClass_Variable; }
    virtual UaNodeId nodeId() const override { return m_nodeId; }
    virtual OpcUa_Int32 valueRank() const { return m_valueRank; }
    virtual void arrayDimensions(UaUInt32Array &    arrayDimensions )   const { arrayDimensions = m_arrayDimensions;}


    const UA_DataValue* valueImpl() const { return m_currentValue.impl(); }
    void associateServer( UA_Server* server );
    void setValueHandling( ValueHandling valueHandling ) { m_valueHandling = valueHandling; }

private:
    UaQualifiedName m_browseName;
    UaDataValue m_currentValue;
    UaNodeId m_nodeId;
    UaNodeId m_typeDefinitionId;
    OpcUa_Int32 m_valueRank;
    OpcUa_Byte m_accessLevel;
    UaUInt32Array m_arrayDimensions;

    /* It seems necessary to comment on ValueHandling policies within open62541-compat; as it is a wrapper layer on top of open62541,
     * attempting to look&feel like UASDK is not necessarily easy.
     * To me (Piotr) I choose the following mapping:
     * None - the m_currentValue is not used at all. setValue() and value() should be overridden by a subclass. Within this class their behaviour is undefined.
     * Cache - the m_currentValue only reflects last value passed to setValue(). setValue() will invoke open62541 writeValues() and value() will invoke
     *   open62541 readValues. This mode seems to match the new implementation of cache-variables in open62541-compat.
     * CacheIsSource - the m_currentValue stores the actual "online" value, open62541 will use value() to sample it. This matches the historic way of doing
     *   cache variables using open62541-compat and remains to be used for simpler DataSource applications in future.
     * CacheIsUpdatedOnRequest - not supported at all in open62541 at this very moment.
     */
    ValueHandling m_valueHandling;

    UaStatus requestWriteToServer (UA_NodeId variableNodeId, const UaDataValue& dataValue);
    UaDataValue requestReadFromServer ();

    UA_Server* m_associatedServer;

};



}

#endif // __OPCUA_BASEDATAVARIABLETYPE_H__
