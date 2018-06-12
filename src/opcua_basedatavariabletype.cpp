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
     m_accessLevel(accessLevel)

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
         m_currentValue = dataValue;
         return OpcUa_Good;
     }
     else
         return OpcUa_BadUserAccessDenied;

 }

 UaDataValue BaseDataVariableType::value(Session* session)
 {
     return m_currentValue.clone();
 }   BaseDataVariableType::BaseDataVariableType(
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
         m_accessLevel(accessLevel)

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
             m_currentValue = dataValue;
             return OpcUa_Good;
         }
         else
             return OpcUa_BadUserAccessDenied;

     }

     UaDataValue BaseDataVariableType::value(Session* session)
     {
         return m_currentValue.clone();
     }

}

