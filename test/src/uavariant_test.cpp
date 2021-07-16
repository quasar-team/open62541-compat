/* © Copyright Ben Farnham, CERN, 2018.  All rights not expressly granted are reserved.
 * nodemanagerbase.cpp
 *
 *  Created on: reated on: Apr 11, 2018
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

#include "uavariant_test.h"
#include "arrays.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>

using std::numeric_limits;

TEST_F(UaVariantTest, testCannotConvertUninitalizedVariant)
{
	OpcUa_Int32 result;
	EXPECT_EQ(OpcUa_Bad, m_testee.toInt32(result)) << "should not be able to convert an uninitialized variant to any type";
}

TEST_F(UaVariantTest, testConvertToSameInternalType)
{
	OpcUa_Int32 int32Value = 123;
	m_testee.setInt32(int32Value);

	OpcUa_Int32 int32Result;
	EXPECT_EQ(OpcUa_Good, m_testee.toInt32(int32Result));
	EXPECT_EQ(int32Value, int32Result);

	OpcUa_UInt64 uint64Value = 456;
	m_testee.setUInt64(uint64Value);

	OpcUa_UInt64 uint64Result;
	EXPECT_EQ(OpcUa_Good, m_testee.toUInt64(uint64Result));
	EXPECT_EQ(uint64Value, uint64Result);

	m_testee.setString(UaString("abcde"));
	EXPECT_EQ("\"abcde\"", m_testee.toString().toUtf8());
}

TEST_F(UaVariantTest, testConvertFromBoolean)
{
	m_testee.setBool(true);

	OpcUa_Boolean booleanResult = false;
	EXPECT_EQ(OpcUa_Good, m_testee.toBool(booleanResult));
	EXPECT_TRUE(booleanResult);

	OpcUa_Byte byteResult;
	EXPECT_EQ(OpcUa_Good, m_testee.toByte(byteResult));
	EXPECT_EQ(1, byteResult);

	OpcUa_Int16 int16Result;
	EXPECT_EQ(OpcUa_Good, m_testee.toInt16(int16Result));
	EXPECT_EQ(1, int16Result);

	OpcUa_UInt32 uint32Result;
	EXPECT_EQ(OpcUa_Good, m_testee.toUInt32(uint32Result));
	EXPECT_EQ(1, uint32Result);

	OpcUa_UInt64 uint64Result;
	EXPECT_EQ(OpcUa_Good, m_testee.toUInt64(uint64Result));
	EXPECT_EQ(1, uint64Result);

	OpcUa_Float floatResult;
	EXPECT_EQ(OpcUa_Good, m_testee.toFloat(floatResult));
	EXPECT_FLOAT_EQ(1, floatResult);

	OpcUa_Double doubleResult;
	EXPECT_EQ(OpcUa_Good, m_testee.toDouble(doubleResult));
	EXPECT_DOUBLE_EQ(1, doubleResult);
}

TEST_F(UaVariantTest, testConvertToBoolean)
{
	OpcUa_Boolean booleanResult;

	m_testee.setBool(true);
	EXPECT_EQ(OpcUa_Good, m_testee.toBool(booleanResult));
	EXPECT_TRUE(booleanResult);

	m_testee.setInt16(1);
	EXPECT_EQ(OpcUa_Good, m_testee.toBool(booleanResult));
	EXPECT_TRUE(booleanResult);

	m_testee.setUInt32(1);
	EXPECT_EQ(OpcUa_Good, m_testee.toBool(booleanResult));
	EXPECT_TRUE(booleanResult);

	m_testee.setInt64(1);
	EXPECT_EQ(OpcUa_Good, m_testee.toBool(booleanResult));
	EXPECT_TRUE(booleanResult);

	m_testee.setFloat(1);
	EXPECT_EQ(OpcUa_Good, m_testee.toBool(booleanResult));
	EXPECT_TRUE(booleanResult);

	m_testee.setDouble(1);
	EXPECT_EQ(OpcUa_Good, m_testee.toBool(booleanResult));
	EXPECT_TRUE(booleanResult);
}

TEST_F(UaVariantTest, testConvertFromInt32)
{
	OpcUa_Byte byteResult;
	OpcUa_Int16 int16Result;
	OpcUa_UInt32 uint32Result;
	OpcUa_UInt64 uint64Result;
	OpcUa_Float floatResult;

	m_testee.setInt32(1);
	EXPECT_EQ(OpcUa_Good, m_testee.toByte(byteResult));
	EXPECT_EQ(1, byteResult);
	EXPECT_EQ(OpcUa_Good, m_testee.toInt16(int16Result));
	EXPECT_EQ(1, int16Result);
	EXPECT_EQ(OpcUa_Good, m_testee.toUInt32(uint32Result));
	EXPECT_EQ(1, uint32Result);
	EXPECT_EQ(OpcUa_Good, m_testee.toUInt64(uint64Result));
	EXPECT_EQ(1, uint64Result);
	EXPECT_EQ(OpcUa_Good, m_testee.toFloat(floatResult));
	EXPECT_FLOAT_EQ(1, floatResult);

	m_testee.setInt32(-1);
	EXPECT_EQ(OpcUa_BadOutOfRange, m_testee.toByte(byteResult))<< "expected to fail: -1 out of range for conversion to unsigned";
	EXPECT_EQ(OpcUa_Good, m_testee.toInt16(int16Result));
	EXPECT_EQ(-1, int16Result);
	EXPECT_EQ(OpcUa_BadOutOfRange, m_testee.toUInt32(uint32Result))<< "expected to fail: -1 out of range for conversion to unsigned";
	EXPECT_EQ(OpcUa_BadOutOfRange, m_testee.toUInt64(uint64Result))<< "expected to fail: -1 out of range for conversion to unsigned";
	EXPECT_EQ(OpcUa_Good, m_testee.toFloat(floatResult));
	EXPECT_FLOAT_EQ(-1, floatResult);

	m_testee.setInt32(numeric_limits<int32_t>::max());
	EXPECT_EQ(OpcUa_BadOutOfRange, m_testee.toByte(byteResult))<< "expected to fail: number too large for byte";
	EXPECT_EQ(OpcUa_BadOutOfRange, m_testee.toInt16(int16Result))<< "expected to fail: number too large for 16 bit integer";
	EXPECT_EQ(OpcUa_Good, m_testee.toUInt32(uint32Result));
	EXPECT_EQ(numeric_limits<int32_t>::max(), uint32Result);
	EXPECT_EQ(OpcUa_Good, m_testee.toUInt64(uint64Result));
	EXPECT_EQ(numeric_limits<int32_t>::max(), uint64Result);
	EXPECT_EQ(OpcUa_Good, m_testee.toFloat(floatResult));
	EXPECT_FLOAT_EQ(numeric_limits<int32_t>::max(), floatResult);
}

TEST_F(UaVariantTest, testConvertFromUInt32)
{
	OpcUa_Int32 int32Result;

	m_testee.setUInt32(1);
	EXPECT_EQ(OpcUa_Good, m_testee.toInt32(int32Result));
	EXPECT_EQ(1, int32Result);
	EXPECT_EQ(OpcUa_Good, m_testee.toInt32(int32Result));
	EXPECT_EQ(1, int32Result);

	m_testee.setUInt32(numeric_limits<uint32_t>::max());
	EXPECT_EQ(OpcUa_BadOutOfRange, m_testee.toInt32(int32Result));

	m_testee.setUInt32(numeric_limits<int32_t>::max());
	EXPECT_EQ(OpcUa_Good, m_testee.toInt32(int32Result));
	EXPECT_EQ(numeric_limits<int32_t>::max(), int32Result);
}
