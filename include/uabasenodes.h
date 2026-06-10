/* © Copyright Piotr Nikiel, CERN, 2017.  All rights not expressly granted are reserved.
 * uabasenodes.h
 *
 *  Created on: 26 Apr, 2017
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

#ifndef OPEN62541_COMPAT_INCLUDE_UABASENODES_H_
#define OPEN62541_COMPAT_INCLUDE_UABASENODES_H_

#include <uanode.h>
#include <statuscode.h>
#include <uadatavalue.h>
#include <session.h>

class UaVariable: public UaNode
{
public:
    virtual UaDataValue value(Session* session) = 0;
    virtual UaStatus setValue(
            Session *session,
            const UaDataValue& dataValue,
            OpcUa_Boolean checkAccessLevel = OpcUa_True ) = 0;
};

class UaObject: public UaNode
{

};

class UaMethod: public UaNode
{
public:
	UaMethod() {}

};


#endif /* OPEN62541_COMPAT_INCLUDE_UABASENODES_H_ */
