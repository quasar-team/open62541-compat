/* © Copyright Piotr Nikiel, CERN, 2017.  All rights not expressly granted are reserved.
 * nodemanagerbase.h
 *
 *  Created on: 23 Apr,  2017
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

#ifndef __METHODHANDLEUANODE_H__
#define __METHODHANDLEUANODE_H__


#include <uabasenodes.h>


class MethodHandle
{
public:
	virtual ~MethodHandle() {};
};

class MethodHandleUaNode: public MethodHandle
{
public:
	virtual ~MethodHandleUaNode() {};
	MethodHandleUaNode():
		m_obj(0),
		m_method(0)
	{}

	UaMethod * 	pUaMethod () const { return m_method; }
	UaObject * 	pUaObject () const { return m_obj; }

	void setUaNodes (UaObject* uaObject, UaMethod* uaMethod)
	{
		m_obj = uaObject;
		m_method = uaMethod;
	}
private:
	UaObject *m_obj;
	UaMethod *m_method;

};

// FIXME wrong file
class UaStatusCodeArray
{

};

// FIXME wrong file
class UaDiagnosticInfos
{

};

#endif // __METHODHANDLEUANODE_H__
