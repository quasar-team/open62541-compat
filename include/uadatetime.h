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


#ifndef __UADATETIME_H_
#define __UADATETIME_H_

#include <open62541.h>

class UaDateTime
{
public:
	UaDateTime();
	static UaDateTime now();

	void addSecs(int secs);
	void addMilliSecs(int msecs);

	static UaDateTime fromString(const UaString&);
	UaString toString() const;

private:
	UaDateTime(const UA_DateTimeStruct& dateTime);

	UA_DateTimeStruct m_dateTime;

	static void initializeInternalDateTimeStruct(UA_DateTimeStruct& dateTimeStruct);
	static void cloneExternalDateTimeStruct(UA_DateTimeStruct& destDateTimeStruct, const UA_DateTimeStruct& srcDateTimeStruct);
};

#endif // __UADATETIME_H_
