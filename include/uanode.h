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


#ifndef __UANODE_H__
#define __UANODE_H__

#include <uanodeid.h>
#include <other.h>
#include <list>

enum OpcUa_NodeClass
{
    OpcUa_NodeClass_Object,
    OpcUa_NodeClass_Variable,
	OpcUa_NodeClass_Method
};



class UaNode
{
public:
    UaNode();
    virtual ~UaNode() {}

    virtual UaNodeId nodeId() const = 0;

    virtual UaQualifiedName browseName() const = 0;
    virtual UaNodeId typeDefinitionId() const = 0;
    virtual OpcUa_NodeClass nodeClass() const = 0;

    struct ReferencedTarget
    {
	UaNode* target;
	UaNodeId referenceTypeId;
    ReferencedTarget( UaNode* aTarget, const UaNodeId& referencedtarget ): target(aTarget), referenceTypeId(referencedtarget) {}
    };

    void releaseReference() {} // TODO: ??

    void addReferencedTarget( UaNode* targetNode, UaNodeId referenceTypeId ) { m_referenceTargets.push_back( ReferencedTarget(targetNode, referenceTypeId));  }
    const std::list<ReferencedTarget>* referencedTargets() const { return &m_referenceTargets; }
private:    
    std::list<ReferencedTarget> m_referenceTargets;

 
};

typedef UaNode UaReferenceLists;

#endif // __UANODE_H__
