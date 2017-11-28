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


#ifndef __UANODEID_H__
#define __UANODEID_H__


#include <uastring.h>
#include <opcua_platformdefs.h>

//TODO: switch for amalgamation
#include <open62541.h>

//FIXME: should base on open62541 constants
enum IdentifierType
{
    OpcUa_IdentifierType_Numeric,
    OpcUa_IdentifierType_String
};


class UaNodeId
{
public:
    //! Create string address
    UaNodeId ( const UaString& stringAddress, int ns);
    UaNodeId ( int numericAddress, int ns);
    UaNodeId ( const UaNodeId& other);
    ~UaNodeId ();
    
    const UaNodeId& operator=(const UaNodeId & other);
    unsigned int namespaceIndex() const { return m_impl.namespaceIndex; }
    UaString identifierString() const;
    unsigned int identifierNumeric() const { return m_impl.identifier.numeric; }
    IdentifierType identifierType() const;
    UaString toString() const;
    UaString toFullString() const { return toString(); } // TODO?
    //! Return Implementation specific data. Note: the data pointed to in the pointers hidden in the structure are valid as long as this object is alive and shall be regarded const.
    UA_NodeId impl() const { return m_impl; }
    const UA_NodeId* pimpl() const { return &m_impl; }

    void copyTo( UA_NodeId* other) const;
    void copyTo( UaNodeId* other) const;
    bool operator==(const UaNodeId& other) const;
private:
    /* UaString m_stringId; */
    UA_NodeId m_impl;

};



#endif // __UANODEID_H__
