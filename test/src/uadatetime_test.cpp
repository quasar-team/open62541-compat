/* © Copyright Ben Farnham, CERN, 2021.  All rights not expressly granted are reserved.
 * nodemanagerbase.cpp
 *
 *  Created on: reated on: Mar 1, 2021
 *      Author: Ben Farnham <ben.farnham@cern.ch>
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

#include "gtest/gtest.h"
#include "uadatetime.h"

TEST(UaDatetimeTest, testDefaultCtor)
{
	UaDateTime testee;
	EXPECT_EQ(0, static_cast<UA_DateTime>(testee)) << "default should be epoch + 0";
}

TEST(UaDatetimeTest, testOpen62541TypeCtor)
{
	UaDateTime testee(123);
	EXPECT_EQ(123, static_cast<UA_DateTime>(testee)) << "ctor arg maps directly onto member";
}

TEST(UaDatetimeTest, testAddSecs)
{
	UaDateTime testee;
	testee.addSecs(1);
	EXPECT_EQ(1E7, static_cast<UA_DateTime>(testee)) << "time stored as 100usec intervals since epoch";
}

TEST(UaDatetimeTest, testAddMilliSecs)
{
	UaDateTime testee;
	testee.addMilliSecs(1);
	EXPECT_EQ(1E4, static_cast<UA_DateTime>(testee)) << "time stored as 100usec intervals since epoch";
}

TEST(UaDatetimeTest, testSecsTo)
{
	UaDateTime earlier(static_cast<UA_DateTime>(1*1E7)); // 1s
	UaDateTime later(static_cast<UA_DateTime>(5*1E7)); // 5s

	EXPECT_EQ(4, earlier.secsTo(later));
	EXPECT_EQ(-4, later.secsTo(earlier));

	later.addMilliSecs(1);
	EXPECT_EQ(4, earlier.secsTo(later)) << "4s difference (complete secs only)";
	
	later.addMilliSecs(500);
	EXPECT_EQ(4, earlier.secsTo(later)) << "4s difference (complete secs only)";

	later.addMilliSecs(500);
	EXPECT_EQ(5, earlier.secsTo(later));
}

TEST(UaDatetimeTest, testFromString)
{
	try
	{
		UaDateTime testee = UaDateTime::fromString(UaString("not a date string"));
		FAIL() << "invalid string should throw";
	}
	catch (const std::exception&)
	{} // expected

	EXPECT_EQ(0, static_cast<UA_DateTime>( UaDateTime::fromString(UaString("1601-01-01T00:00:00Z")) )) << "OPC-UA epoch should be windows epoch";
	EXPECT_EQ(UA_DATETIME_SEC, static_cast<UA_DateTime>( UaDateTime::fromString(UaString("1601-01-01T00:00:01Z")) )) << "expect 1s after epoch";

	const UA_DateTimeStruct datetimeStruct = UA_DateTime_toStruct(static_cast<UA_DateTime>(UaDateTime::fromString("2026-07-02T17:55:00.123456789Z")));
	EXPECT_EQ(2026, datetimeStruct.year);
	EXPECT_EQ(7, datetimeStruct.month);
	EXPECT_EQ(2, datetimeStruct.day);
	EXPECT_EQ(17, datetimeStruct.hour);
	EXPECT_EQ(55, datetimeStruct.min);
	EXPECT_EQ(0, datetimeStruct.sec);
	EXPECT_EQ(123, datetimeStruct.milliSec);
	EXPECT_EQ(456, datetimeStruct.microSec);
	EXPECT_TRUE(datetimeStruct.nanoSec == 700 || datetimeStruct.nanoSec == 800) << "789ns should be converted to OPC 100ns granularity; 700 or 800ns acceptable";
}
