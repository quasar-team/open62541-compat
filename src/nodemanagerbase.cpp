/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
 * nodemanagerbase.cpp
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


#include <nodemanagerbase.h>
#include <boost/foreach.hpp>
#include <iostream>
#include <opcua_basedatavariabletype.h>
#include <ASUtils.h>

NodeManagerBase::NodeManagerBase( const char* uri, bool sth, int hashtablesize ):
  m_server(0),
  m_nameSpaceUri(uri)
{

}

NodeManagerBase::~NodeManagerBase()
{


    std::cout << __PRETTY_FUNCTION__ << std::endl;
    std::cout << "m_listNodes.size=" << m_listNodes.size() << std::endl;
    while (!m_listNodes.empty())
    {
	auto node = m_listNodes.back();
	m_listNodes.pop_back();

	delete node;
    }
    
}

UaNode* NodeManagerBase::getNode( const UaNodeId& nodeId ) const
{
    //TODO: the code belove is probably shitty - shall be decided one and forever whether getNode shall be const or not ...
    if (nodeId == m_serverRootNode.nodeId())
	return (UaNode*)(&m_serverRootNode);

    BOOST_FOREACH (UaNode* node,  m_listNodes)
    {
	if (node->nodeId() == nodeId)
	    return node;
    }
    return 0; // not found
}


static UA_StatusCode unifiedRead(void *handle, const UA_NodeId nodeid, UA_Boolean sourceTimeStamp, const UA_NumericRange *range, UA_DataValue *dataValue) 
{
    // we expect that the handle points to an object of subclass of BaseDataVariableType
    OpcUa::BaseDataVariableType *variable = static_cast<OpcUa::BaseDataVariableType*>(handle);
    std::cout << "static cast is " << variable << std::endl;
    std::cout << "read request comes from " << variable->nodeId().toString().toUtf8() << std::endl;
    UA_DataValue_copy( variable->valueImpl(), dataValue );
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode unifiedWrite(void *handle, const UA_NodeId nodeid, const UA_Variant *data, const UA_NumericRange *range)
{
    if ( !data )
    {
        std::cout << "unifiedWrite data=" << data << std::endl;
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    if ( !handle )
    {
        std::cout << "unifiedWrite handle=0" << std::endl;
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    // we expect that the handle points to an object of subclass of BaseDataVariableType
    OpcUa::BaseDataVariableType *variable = static_cast<OpcUa::BaseDataVariableType*>(handle);
    std::cout << "write request" << std::endl;
    UaVariant variant ( *data );
    UaStatus status = variable->setValue( /*anything non zero*/(Session*)-1, UaDataValue( variant, OpcUa_Good, UaDateTime::now(), UaDateTime::now() ), OpcUa_True );
    return status;
}


    UaString localisationCode("en_US");
    
UA_LocalizedText make_localised( UaString text )
{
    UA_LocalizedText out;
    out.locale = *localisationCode.impl();
    out.text = *(new UaString(text))->impl();
    return out;
}
    
UaStatus NodeManagerBase::addNodeAndReference( 
    UaNode* parent,
    UaNode* to,
    const UaNodeId& refType)
{
	std::cout << __FILE__ << " " << __LINE__ << std::endl;
   // std::cout << __PRETTY_FUNCTION__ << "from.id=" << from.toString().toUtf8() << " to.id=" << to->nodeId().toString().toUtf8() <<  std::endl;
    // std::cout << "ref=" << refType.toString().toUtf8() << endl;
    UaLocalizedText displayName( "en_US", to->browseName().unqualifiedName().toUtf8().c_str());
	
    switch( to->nodeClass() )
    {
    case OpcUa_NodeClass_Object:
	{
	    UA_ObjectAttributes objectAttributes;
	    UA_ObjectAttributes_init( &objectAttributes );
	    objectAttributes.description = UA_LOCALIZEDTEXT("en_US","the answer");
	    objectAttributes.displayName = *displayName.impl();
	    UA_StatusCode s = UA_Server_addObjectNode(
		/*server*/ m_server,
		/*newnodeid*/ to->nodeId().impl(),
		/*parentid*/ parent->nodeId().impl(),
		/*ref id*/ refType.impl(),
		/*browsename*/ to->browseName().impl(),
		/*type def*/ to->typeDefinitionId().impl(),
		/* object attrs*/ objectAttributes,
                /* instantiation cbk*/ 0,
		/*out new node id*/ 0
		);    
	    std::cout << "UA_Server_addObjectNode() finished with code " << std::hex << s << std::dec << std::endl;
	    if (UA_STATUSCODE_GOOD == s)
	    {
		m_listNodes.push_back( to );
		parent->addReferencedTarget( to, refType );
	    }
	    return s;
	}
    case OpcUa_NodeClass_Variable:
	{
		// UA_DataSource dateDataSource = (UA_DataSource) {
	    // 	.handle = &myInteger, .read = readInteger, .write = writeInteger};
	    UA_DataSource dateDataSource 
	    {
		static_cast<void*>(to),
		    unifiedRead,
		    unifiedWrite
		    };
	    UA_VariableAttributes attr;
	    UA_VariableAttributes_init(&attr);
 	    attr.description = UA_LOCALIZEDTEXT("en_US","the answer");
	    attr.displayName = *displayName.impl();
	    UA_StatusCode s =
		UA_Server_addDataSourceVariableNode(m_server,
						    to->nodeId().impl(),
						    parent->nodeId().impl(),
						    refType.impl(),
						    to->browseName().impl(),
						    to->typeDefinitionId().impl(),
						    attr,
						    dateDataSource,
						    NULL
		    );
 	    if (UA_STATUSCODE_GOOD == s)
	    {
		m_listNodes.push_back( to );
		parent->addReferencedTarget( to, refType );
	    }


	    return s;
	}
		
    }

}


UaStatus NodeManagerBase::addNodeAndReference( 
    const UaNodeId& from,
    UaNode* to,
    const UaNodeId& refType)
{
    UaNode* parentNode = getNode (from);
    if (!parentNode)
        return UA_STATUSCODE_BADNODEIDUNKNOWN;
    return addNodeAndReference( parentNode, to, refType);
}


void NodeManagerBase::linkServer( UA_Server* server )
{
	const int nsIndex = UA_Server_addNamespace( m_server, m_nameSpaceUri.c_str() );
	if (nsIndex != 2)
		throw std::logic_error("UA_Server_addNamespace: namespace added to nsindex different than 2. ");
}
