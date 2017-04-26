/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
 * nodemanagerbase.h
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



#ifndef __NODEMANAGERBASE_H__
#define __NODEMANAGERBASE_H__

#include <statuscode.h>
#include <uanode.h>
#include <other.h>





class NodeManagerConfig
{

};


class NodeManagerBase: public NodeManagerConfig
{
public:
	NodeManagerBase( const char* uri, bool sth, int hashtablesize );
	virtual ~NodeManagerBase();
	UaNode* getNode( const UaNodeId& nodeId ) const;

	OpcUa_UInt16 getNameSpaceIndex 	() 	const { return 2; }

	UaStatus addNodeAndReference( 
			const UaNodeId& from,
			UaNode* to,
			const UaNodeId& refType);
	UaStatus addNodeAndReference(
			UaNode* from,
			UaNode* to,
			const UaNodeId& refType); // { return addNodeAndReference( from->nodeId(), to, refType ); }

	void linkServer( UA_Server* server );


private:
	UA_Server* m_server;
	std::list<UaNode*> m_listNodes;
	std::string m_nameSpaceUri;

		class ServerRootNode: public UaNode
		{
		public:
			ServerRootNode() {}
			virtual UaQualifiedName browseName() const { return UaQualifiedName(0, "."); }
			virtual UaNodeId typeDefinitionId() const { return UaNodeId(UA_NS0ID_BASEOBJECTTYPE,0); }
			virtual OpcUa_NodeClass nodeClass() const { return OpcUa_NodeClass::OpcUa_NodeClass_Object; }
			virtual UaNodeId nodeId() const { return UaNodeId(OpcUaId_ObjectsFolder, 0); }
		private:
			ServerRootNode( const ServerRootNode& other );
			void operator=( const ServerRootNode& other );

		};
		ServerRootNode m_serverRootNode;


	};



#endif // __NODEMANAGERBASE_H__
