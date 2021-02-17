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

#include <algorithm>
#include <stdexcept>

#include <nodemanagerbase.h>
#include <opcua_basedatavariabletype.h>
#include <uadatavariablecache.h>
#include <open62541.h>

static UaLocalizedText emptyDescription( "en_US", "" );

NodeManagerBase::NodeManagerBase(
        const char* sNamespaceUri,
        bool        firesEvents,
        OpcUa_Int32 nHashTableSize):
    m_server(nullptr),
    m_nameSpaceUri(sNamespaceUri)
{

}

NodeManagerBase::~NodeManagerBase()
{
    while (!m_listNodes.empty())
    {
        auto node = m_listNodes.back();
        m_listNodes.pop_back();
        delete node;
    }
}

UaNode* NodeManagerBase::getNode( const UaNodeId& nodeId ) const
{
    if (nodeId == m_serverRootNode.nodeId())
        return (UaNode*)(&m_serverRootNode);

    auto iterator = std::find_if(
            m_listNodes.begin(),
            m_listNodes.end(),
            [nodeId](UaNode* node){return node->nodeId() == nodeId;});
    return (iterator != m_listNodes.end()) ? *iterator : nullptr;
}

static UA_StatusCode unifiedRead(
    UA_Server *server,
    const UA_NodeId *sessionId,
    void *sessionContext,
    const UA_NodeId *nodeId,
    void *nodeContext,
    UA_Boolean includeSourceTimeStamp,
    const UA_NumericRange *range,
    UA_DataValue *dataValue)



{
    // we expect that the handle points to an object of subclass of BaseDataVariableType -- cause it's how we add then
    OpcUa::BaseDataVariableType *variable = static_cast<OpcUa::BaseDataVariableType*>(nodeContext);

    UaDataValue aCopy( variable->value(0) ); // internally cloned so we can do anything with this object
    UA_DataValue_copy( aCopy.impl(), dataValue );

    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode unifiedWrite(
    UA_Server *server,
    const UA_NodeId *sessionId,
    void *sessionContext,
    const UA_NodeId *nodeId,
    void *nodeContext,
    const UA_NumericRange *range,
    const UA_DataValue *dataValue
)
{
    if ( !dataValue )
    {
        LOG(Log::ERR) << "unifiedWrite data=" << dataValue;
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    if ( !nodeContext )
    {
        LOG(Log::ERR) << "nodeContext is null";
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    // we expect that the handle points to an object of subclass of BaseDataVariableType
    OpcUa::BaseDataVariableType *variable = static_cast<OpcUa::BaseDataVariableType*>(nodeContext);
    UaVariant variant ( dataValue->value );
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

    UaStatus getStatusCode () const {
        return m_resultStatus;
    }
    UaVariantArray& outputs() {
        return m_outputs;
    }

private:
    UaStatus m_resultStatus;
    UaVariantArray m_outputs;

};


//! IMPORTANT: in adding methods we pass nodeContext but where is this routed to in the end? To methodContext or objectContext?
UA_StatusCode unifiedCall(
    UA_Server *server,
    const UA_NodeId *sessionId,
    void *sessionContext,
    const UA_NodeId *methodId,
    void *methodContext,
    const UA_NodeId *objectId,
    void *objectContext,
    size_t inputSize,
    const UA_Variant *input,
    size_t outputSize,
    UA_Variant *output
)
{
    LOG(Log::TRC) << "Method called! handle=" << methodContext << " input arguments size=" << inputSize;
    MethodHandleUaNode *handle = static_cast<MethodHandleUaNode*> (methodContext);
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

UaStatus NodeManagerBase::addObjectNodeAndReference(
        UaNode* parent,
        UaNode* to,
        const UaNodeId& refType)
{
    UaLocalizedText displayName( "en_US", to->browseName().unqualifiedName().toUtf8().c_str());
    UA_ObjectAttributes objectAttributes;
    UA_ObjectAttributes_init( &objectAttributes );
    objectAttributes.description = *emptyDescription.impl();
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

    LOG(Log::TRC) << "obtained output: ns=" << out.namespaceIndex << "," << UaString(&out.identifier.string).toUtf8();
    if (UA_STATUSCODE_GOOD == s)
    {
        m_listNodes.push_back( to );
        parent->addReferencedTarget( to, refType );
    }
    else
        LOG(Log::ERR) << "Failed to add new node: " << std::hex << s;
    return s;

}

/** Piotr's comment: this mechanism was used historically to handle cache-variables in open62541-compat.
 * However, it is no longer needed for cache-variables as we have better ways of doing it, using proper Variables.
 * I'm not deprecating it though because it very well matches what's needed for synchronous source-variables.
 */
UaStatus NodeManagerBase::addDataSourceVariableNodeAndReference(
        UaNode* parent,
		OpcUa::BaseDataVariableType* variable,
        const UaNodeId& refType)
{
	UaLocalizedText displayName( "en_US", variable->browseName().unqualifiedName().toUtf8().c_str());
    UA_DataSource dateDataSource
    {
        unifiedRead,
        unifiedWrite
    };
    UA_VariableAttributes attr;
    UA_VariableAttributes_init(&attr);
    attr.description = *emptyDescription.impl();
    attr.displayName = *displayName.impl();
    attr.dataType = variable->typeDefinitionId().impl();
    attr.valueRank = variable->valueRank();
    attr.accessLevel = variable->accessLevel();
    UaUInt32Array arrayDimensions;
    variable->arrayDimensions(arrayDimensions);
    attr.arrayDimensionsSize = arrayDimensions.size();
    if (arrayDimensions.size() > 0)
        attr.arrayDimensions = &arrayDimensions[0];
    else
        attr.arrayDimensions = nullptr;
    variable->value(/*session*/nullptr).value()->copyTo(&attr.value);
    UA_StatusCode s =
        UA_Server_addDataSourceVariableNode(m_server,
                                            variable->nodeId().impl(),
                                            parent->nodeId().impl(),
                                            refType.impl(),
                                            variable->browseName().impl(),
                                            UaNodeId(UA_NS0ID_BASEDATAVARIABLETYPE ,0).impl() ,
                                            attr,
                                            dateDataSource,
                                            static_cast<void*>(variable),  // this is our nodeContext - we'll use it to map to the variable
                                            nullptr
                                           );
    if (UA_STATUSCODE_GOOD == s)
    {
        m_listNodes.push_back( variable );
        parent->addReferencedTarget( variable, refType );
    }
    return s;
}

UaStatus NodeManagerBase::addDataVariableNodeAndReference(
        UaNode* parent,
		OpcUa::BaseDataVariableType* variable,
        const UaNodeId& refType)
{
	UaLocalizedText displayName( "en_US", variable->browseName().unqualifiedName().toUtf8().c_str());

    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.description = *emptyDescription.impl();
    attr.displayName = *displayName.impl();
    attr.dataType = variable->typeDefinitionId().impl();
    attr.valueRank = variable->valueRank();
    attr.accessLevel = variable->accessLevel();
    UaUInt32Array arrayDimensions;
    variable->arrayDimensions(arrayDimensions);
    attr.arrayDimensionsSize = arrayDimensions.size();
    if (arrayDimensions.size() > 0)
        attr.arrayDimensions = &arrayDimensions[0];
    else
        attr.arrayDimensions = nullptr;
    variable->value(/*session*/nullptr).value()->copyTo(&attr.value);
    UA_StatusCode s =
        UA_Server_addVariableNode(  m_server,
									variable->nodeId().impl(),
									parent->nodeId().impl(),
									refType.impl(),
									variable->browseName().impl(),
									UaNodeId(UA_NS0ID_BASEDATAVARIABLETYPE ,0).impl() ,
									attr,
									nullptr, /* nodecontext */
									nullptr /* output nodeid - not using automatic assignment so don't care */
								   );
    if (UA_STATUSCODE_GOOD == s)
    {
        m_listNodes.push_back( variable );
        parent->addReferencedTarget( variable, refType );
    }
    variable->associateServer(m_server);
    variable->setValueHandling(ValueHandling::UaVariable_Value_Cache);
    return s;
}

UaStatus NodeManagerBase::addVariableNodeAndReference(
    UaNode* parent,
    UaNode* to,
    const UaNodeId& refType)
{
    if (dynamic_cast<UaPropertyMethodArgument*>(to))
    {
        // We don't add UaPropertyMethodArguments to the Address Space when open62541-compat is in use
        // because in the open62541 such properties are directly coupled to (and created by!) methods they belong to.
        parent->addReferencedTarget(to, refType);
        return OpcUa_Good;
    }
    else if (auto* cast = dynamic_cast<UaPropertyCache*>(to))
    {
    	return addPropertyNodeAndReference(parent, cast, refType);
    }
    else if (auto* cast = dynamic_cast<OpcUa::BaseDataVariableType*>(to))
    {
    	return addDataVariableNodeAndReference(parent, cast, refType);
    }
    else
    	OPEN62541_COMPAT_LOG_AND_THROW(
    			std::logic_error,
				"Can't add this variable: no obvious handler in existing open62541 interface, do not know what to do! Requested variable address is " + to->nodeId().toString().toUtf8());
}

UaStatus NodeManagerBase::addPropertyNodeAndReference(
    UaNode* parent,
	UaPropertyCache* to,
    const UaNodeId& refType)
{
	UaLocalizedText displayName( "en_US", to->browseName().unqualifiedName().toUtf8().c_str());
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = *displayName.impl();
    to->value(nullptr).value()->copyTo(&attr.value);

    LOG(Log::TRC) << "to=" << to->nodeId().toFullString().toUtf8() << " reftype=" << refType.toFullString().toUtf8();
	UaStatus s =
			UA_Server_addVariableNode(
					m_server,
					to->nodeId().impl(),
					parent->nodeId().impl(),
					refType.impl(),
					to->browseName().impl(),
					UaNodeId(UA_NS0ID_PROPERTYTYPE, 0).impl(),
					attr,
					/* nodeContext */ nullptr,
					/* outNewNodeId */ nullptr);
	if (!s.isGood())
		OPEN62541_COMPAT_LOG_AND_THROW(
				std::runtime_error,
				"failed to add property " + to->nodeId().toString().toUtf8() + " because: " + s.toString().toUtf8());
	return s;
}

UaStatus NodeManagerBase::addMethodNodeAndReference(
    UaNode* parent,
    UaNode* to,
    const UaNodeId& refType)
{
    UaLocalizedText displayName( "en_US", to->browseName().unqualifiedName().toUtf8().c_str());
    UA_MethodAttributes attr;
    UA_MethodAttributes_init(&attr);
    attr.executable = true;
    attr.userExecutable = true;
    attr.displayName = *displayName.impl();
    attr.description = *emptyDescription.impl();

    MethodHandleUaNode *handle = new MethodHandleUaNode;
    handle->setUaNodes( static_cast<UaObject*>(parent), static_cast<UaMethod*>(to) );

    LOG(Log::TRC) << "parent node: " << parent->nodeId().toFullString().toUtf8();

    const std::list<UaNode::ReferencedTarget>* referenced =  to->referencedTargets();
    LOG(Log::TRC) << "Referenced nodes: " << referenced->size();

    UA_Argument *inArgs = 0;
    int inArgsSize = 0;
    UA_Argument *outArgs = 0;
    int outArgsSize = 0;

    for ( std::list<UaNode::ReferencedTarget>::const_iterator it = referenced->cbegin(); it!=referenced->cend(); it++ )
    {
        const UaNode::ReferencedTarget& refTarget = *it;
        if (refTarget.referenceTypeId == UaNodeId(OpcUaId_HasProperty, 0))
        {
            const UaPropertyMethodArgument* property = dynamic_cast<const UaPropertyMethodArgument*> ( refTarget.target );
            if (property->argumentType() == UaPropertyMethodArgument::INARGUMENTS)
            {
                inArgsSize = property->numArguments();
                inArgs = new UA_Argument[ property->numArguments() ];
                for (unsigned int i=0; i < property->numArguments(); ++i )
                {
                    inArgs[i] = property->implArgument(i);
                }
            }
            else
            {
                outArgsSize = property->numArguments();
                outArgs = new UA_Argument[ property->numArguments() ];
                for (unsigned int i=0; i < property->numArguments(); ++i )
                {
                    outArgs[i] = property->implArgument(i);
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
            /*size_t inputArgumentsSize*/ inArgsSize,
            /*const UA_Argument* inputArguments*/ inArgs,
            /*size_t outputArgumentsSize*/ outArgsSize,
            /*const UA_Argument* outputArguments*/ outArgs,
            /*void *nodeContext */ (void*)handle,
            nullptr);
    if (! s.isGood())
    {
    	OPEN62541_COMPAT_LOG_AND_THROW(
    			std::runtime_error,
				"failed to add the method node:"+std::string(s.toString().toUtf8()));
    }
    m_listNodes.push_back( to );
    parent->addReferencedTarget( to, refType );
    return OpcUa_Good;

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
        return addObjectNodeAndReference (parent, to, refType);
    case OpcUa_NodeClass_Variable:
        return addVariableNodeAndReference(parent, to, refType);
    case OpcUa_NodeClass_Method:
        return addMethodNodeAndReference(parent, to, refType);
    default:
        OPEN62541_COMPAT_LOG_AND_THROW(
			std::runtime_error,
			"Adding this nodeClass to the address space is not yet implemented by open62541-compat.");
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

UaStatus NodeManagerBase::addUaReference(
    const UaNodeId& from,
    const UaNodeId& to,
    const UaNodeId& refType)
{
	LOG(Log::INF) << "addUaReference: from: " << from.toFullString().toUtf8() << ", to: " << to.toFullString().toUtf8() << ", refType: " << refType.toFullString().toUtf8();
	// resolve namespace id.
	// this is where we had issues ;-) seems addUaReference can only work within the same server.

//	UA_ExpandedNodeId expandedTarget;
//	expandedTarget.nodeId = to.impl();
//	expandedTarget.namespaceUri = *UaString("adas").impl();
//	expandedTarget.serverIndex = 1;
//
//	UaStatus s = UA_Server_addReference(
//			m_server,
//			from.impl(),
//			refType.impl(),
//			expandedTarget,
//			OpcUa_True /*isForward*/);
//	LOG(Log::INF) << "addUaReference returned: " << s.toString().toUtf8();
//	return s;
    OPEN62541_COMPAT_LOG_AND_THROW(
		std::runtime_error,
		"addUaReference is not yet well implemented, implement me.");
}

void NodeManagerBase::linkServer( UA_Server* server )
{
    m_server = server;
    const int nsIndex = UA_Server_addNamespace( m_server, m_nameSpaceUri.c_str() );
    if (nsIndex != 2)
    	OPEN62541_COMPAT_LOG_AND_THROW(std::logic_error,
    			"UA_Server_addNamespace: namespace added to nsindex different than 2 (it must be 2 for the moment due to simplified compatibility with UA-SDK");
}

UaStatus NodeManagerBase::afterStartUp()
{
    return OpcUa_Good;
}
