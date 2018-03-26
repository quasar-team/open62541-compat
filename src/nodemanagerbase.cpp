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
#include <stdexcept>
#include <uadatavariablecache.h>

NodeManagerBase::NodeManagerBase( const char* uri, bool sth, int hashtablesize ):
  m_server(0),
  m_nameSpaceUri(uri)
{

}

NodeManagerBase::~NodeManagerBase()
{


    std::cout << __FUNCTION__ << std::endl;
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

    UaDataValue aCopy( variable->value(0) ); // internally cloned so we can do anything with this object
    UA_DataValue_copy( aCopy.impl(), dataValue );

    // Piotr: I think that this version of open6 is leaking, try to use this code and you will see:
    // dataValue->hasValue = true;
    // double v = rand();
    // UA_Variant_setScalarCopy(&dataValue->value, &v, &UA_TYPES[UA_TYPES_DOUBLE]);
    
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
    UaVariant variant ( *data );
    UaStatus status = variable->setValue( /*anything non zero*/(Session*)-1, UaDataValue( variant, OpcUa_Good, UaDateTime::now(), UaDateTime::now() ), OpcUa_True );
    return status;
}

class SynchronousMethodCallback: public MethodManagerCallback
{
public:
	virtual ~SynchronousMethodCallback () {}

	virtual UaStatus 	finishCall (
			OpcUa_UInt32 callbackHandle,
			UaStatusCodeArray &inputArgumentResults,
			UaDiagnosticInfos &inputArgumentDiag,
			UaVariantArray &outputArguments,
			UaStatus &statusCode)
	{
		// TODO: store the answer
		this->m_resultStatus = statusCode;
		m_outputs = outputArguments;
		return OpcUa_Good;
	}

	UaStatus getStatusCode () const { return m_resultStatus; }
	UaVariantArray& outputs() { return m_outputs; }

private:
	UaStatus m_resultStatus;
	UaVariantArray m_outputs;

};

UA_StatusCode unifiedCall(
		void *methodHandle,
		const UA_NodeId objectId,
        size_t inputSize,
		const UA_Variant *input,
        size_t outputSize,
		UA_Variant *output)
{
	LOG(Log::INF) << "called! handle=" << methodHandle << " size=" << inputSize;
	MethodHandleUaNode *handle = static_cast<MethodHandleUaNode*> (methodHandle);
	OpcUa::BaseObjectType *receiver = static_cast<OpcUa::BaseObjectType*> ( handle->pUaObject() );

	ServiceContext sc;
	UaVariantArray inputArgs;

	inputArgs.create( inputSize );
	for (size_t i=0; i<inputSize; ++i)
	{
		inputArgs[i] = UaVariant( input[i] );
	}

	SynchronousMethodCallback synchronousCallback;

	UaStatus status =
	receiver->beginCall(
			&synchronousCallback  /* callback */,
			sc /* fake service context */,
			0 /* fake callback handle */,
			handle /* fake method handle */,
			inputArgs
			);

	if (status.isNotGood())
		return status; // beginning failed ...



	for (size_t i=0; i<outputSize; ++i)
	{
		const UA_Variant* from = synchronousCallback.outputs()[i].impl();
		UA_Variant_copy(from, output+i);
	}


	return synchronousCallback.getStatusCode();
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
    UaLocalizedText displayName( "en_US", to->browseName().unqualifiedName().toUtf8().c_str());
    UaLocalizedText dummyDescription( "en_US", "DummyDescription" );
    switch( to->nodeClass() )
    {
    case OpcUa_NodeClass_Object:
	{
	    UA_ObjectAttributes objectAttributes;
	    UA_ObjectAttributes_init( &objectAttributes );
	    objectAttributes.description = *dummyDescription.impl();
	    objectAttributes.displayName = *displayName.impl();
	    UA_NodeId out;
	    UA_StatusCode s = UA_Server_addObjectNode(
		/*server*/ m_server,
		/*newnodeid*/ to->nodeId().impl(),
		/*parentid*/ parent->nodeId().impl(),
		/*ref id*/ refType.impl(),
		/*browsename*/ to->browseName().impl(),
		/*type def*/ to->typeDefinitionId().impl(),
		/* object attrs*/ objectAttributes,
                /* instantiation cbk*/ 0,
		/*out new node id*/ &out
		);    

	    LOG(Log::INF) << "obtained output: ns=" << out.namespaceIndex << "," << UaString(&out.identifier.string).toUtf8();
	    if (UA_STATUSCODE_GOOD == s)
	    {
			m_listNodes.push_back( to );
			parent->addReferencedTarget( to, refType );
	    }
	    else
	    	LOG(Log::ERR) << "Failed to add new node: " << std::hex << s;
	    return s;
	}
    case OpcUa_NodeClass_Variable:
	{
		// skip HasProperty

		if (refType == OpcUaId_HasProperty)
		{
			// We don't add Properties to the Address Space when open62541-compat is in use
			// open62541 does it differently: when you add a method, then you specify the properties
			parent->addReferencedTarget(to, refType);
			return OpcUa_Good;

		}

	    UA_DataSource dateDataSource 
	    {
		static_cast<void*>(to),
		    unifiedRead,
		    unifiedWrite
		    };
	    UA_VariableAttributes attr;
	    UA_VariableAttributes_init(&attr);
 	    attr.description = *dummyDescription.impl();
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

    case OpcUa_NodeClass_Method:
    {
    	UA_MethodAttributes attr;
    	UA_MethodAttributes_init(&attr);
    	attr.executable = true;
    	attr.userExecutable = true;
    	attr.displayName = *displayName.impl();
    	attr.description = *dummyDescription.impl();

    	MethodHandleUaNode *handle = new MethodHandleUaNode;
    	handle->setUaNodes( static_cast<UaObject*>(parent), static_cast<UaMethod*>(to) );

    	LOG(Log::INF) << "parent node: " << parent->nodeId().toFullString().toUtf8();

    	const std::list<UaNode::ReferencedTarget>* referenced =  to->referencedTargets();
    	LOG(Log::INF) << "Referenced nodes: " << referenced->size();

    	UA_Argument *inArgs = 0;
    	int inArgsSize = 0;
    	UA_Argument *outArgs = 0;
    	int outArgsSize = 0;

    	for ( std::list<UaNode::ReferencedTarget>::const_iterator it = referenced->cbegin(); it!=referenced->cend(); it++ )
    	{
    		const UaNode::ReferencedTarget& refTarget = *it;
    		if (refTarget.referenceTypeId == OpcUaId_HasProperty)
    		{
    			const UaPropertyMethodArgument* property = dynamic_cast<const UaPropertyMethodArgument*> ( refTarget.target );
    			if (property->argumentType() == UaPropertyMethodArgument::INARGUMENTS)
    			{
					inArgsSize = property->numArguments();
					inArgs = new UA_Argument[ property->numArguments() ];
					for (unsigned int i=0; i < property->numArguments(); ++i )
					{
						inArgs[i] = property->implArgument(i);
						//LOG(Log::INF) << "Configured: " << inArgs[i].name.data;
					}
    			}
    			else
    			{
					outArgsSize = property->numArguments();
					outArgs = new UA_Argument[ property->numArguments() ];
					for (unsigned int i=0; i < property->numArguments(); ++i )
					{
						outArgs[i] = property->implArgument(i);
						//LOG(Log::INF) << "Configured: " << outArgs[i].name.data;
					}
    			}

    		}

    	}

    	UaStatus s =
    	UA_Server_addMethodNode(
    			m_server,
				to->nodeId().impl(),
				parent->nodeId().impl(),
				refType.impl(),
				to->browseName().impl(),
				attr,
    	        unifiedCall,
				/*void *handle*/ (void*)handle,
    	        /*size_t inputArgumentsSize*/ inArgsSize,
    	        /*const UA_Argument* inputArguments*/ inArgs,
    	        /*size_t outputArgumentsSize*/ outArgsSize,
				/*const UA_Argument* outputArguments*/ outArgs,
    	        NULL);
    	if (! s.isGood())
    	{
    	    throw std::runtime_error("failed to add the method node:"+std::string(s.toString().toUtf8()));
    	}
		m_listNodes.push_back( to );
		parent->addReferencedTarget( to, refType );
    	return 0;
    };

    default:
    	throw std::runtime_error("not-impl");
		
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
	m_server = server;
	const int nsIndex = UA_Server_addNamespace( m_server, m_nameSpaceUri.c_str() );
	if (nsIndex != 2)
		throw std::logic_error("UA_Server_addNamespace: namespace added to nsindex different than 2. ");
}
