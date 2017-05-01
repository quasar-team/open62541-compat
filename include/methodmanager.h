/* © Copyright Piotr Nikiel, CERN, 2017.  All rights not expressly granted are reserved.
 * methodmanager.h
 *
 *  Created on: Apr 18, 2017
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

#ifndef OPEN62541_COMPAT_INCLUDE_METHODMANAGER_H_
#define OPEN62541_COMPAT_INCLUDE_METHODMANAGER_H_

#include <uavariant.h>

    // temporary
	// FIXME
    class UaVariantArray
    {
    public:
    	const UaVariant& operator[](unsigned int index) const { return m_data[index]; }
    	UaVariant& operator[](unsigned int index) { return m_data[index]; }

    private:
    	UaVariant m_data[100];

    };

    class ServiceContext
    {

    };


class MethodManagerCallback
{

public:
	virtual UaStatus 	finishCall (
			OpcUa_UInt32 callbackHandle,
			UaStatusCodeArray &inputArgumentResults,
			UaDiagnosticInfos &inputArgumentDiag,
			UaVariantArray &outputArguments,
			UaStatus &statusCode)=0;
	virtual ~MethodManagerCallback() {};


};

class MethodManager
{
public:
	virtual ~MethodManager() {};
	virtual UaStatus beginCall (
			MethodManagerCallback *callback,
			const ServiceContext  &context,
			OpcUa_UInt32          callbackHandle,
			MethodHandle          *methodHandle,
			const UaVariantArray  &inputArguments)=0;

};





#endif /* OPEN62541_COMPAT_INCLUDE_METHODMANAGER_H_ */
