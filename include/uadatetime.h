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
#include <uastring.h>

class UaDateTime
{
public:
	UaDateTime();
	UaDateTime(const UA_DateTime& dateTime);

	static UaDateTime now();

	void addSecs(int secs);
	void addMilliSecs(int msecs);
	OpcUa_Int32 secsTo(const UaDateTime&) const;

	static UaDateTime fromString(const UaString&);
	UaString toString() const;
	explicit operator UA_DateTime() const {return m_dateTime;}

	static UaDateTime fromTime_t(time_t epochSeconds)
	{ return UaDateTime(static_cast<UA_DateTime>(epochSeconds) * UA_DATETIME_SEC + UA_DATETIME_UNIX_EPOCH); }
	time_t toTime_t() const
	{ return static_cast<time_t>((m_dateTime - UA_DATETIME_UNIX_EPOCH) / UA_DATETIME_SEC); }

private:


	UA_DateTime m_dateTime; // (64bit signed int ) num of 100 nanosec intervals since windows epoch (1601-01-01T00:00:00)
};

typedef UA_DateTime OpcUa_DateTime;

#endif // __UADATETIME_H_
