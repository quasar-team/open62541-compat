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

TEST_F(UaVariantTest, testUaVariants_convertToSameInternalType)
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
}

TEST_F(UaVariantTest, testUaVariants_convertFromInt32)
{
	OpcUa_Byte byteResult;
	OpcUa_UInt32 uint32Result;
	OpcUa_UInt64 uint64Result;
	OpcUa_Float floatResult;

	m_testee.setInt32(0);
	EXPECT_EQ(OpcUa_Good, m_testee.toByte(byteResult));
	EXPECT_EQ(0, byteResult);
	EXPECT_EQ(OpcUa_Good, m_testee.toUInt32(uint32Result));
	EXPECT_EQ(0, uint32Result);
	EXPECT_EQ(OpcUa_Good, m_testee.toUInt64(uint64Result));
	EXPECT_EQ(0, uint64Result);
	EXPECT_EQ(OpcUa_Good, m_testee.toFloat(floatResult));
	EXPECT_EQ(0, floatResult);

	m_testee.setInt32(-1);
	EXPECT_EQ(OpcUa_BadOutOfRange, m_testee.toUInt32(uint32Result));

	m_testee.setInt32(numeric_limits<int32_t>::max());
	EXPECT_EQ(OpcUa_Good, m_testee.toUInt32(uint32Result));
	EXPECT_EQ(static_cast< uint32_t >(numeric_limits<int32_t>::max()), uint32Result);
}

TEST_F(UaVariantTest, testUaVariants_convertFromUInt32)
{
	OpcUa_Int32 int32Result;

	m_testee.setUInt32(0);
	EXPECT_EQ(OpcUa_Good, m_testee.toInt32(int32Result));
	EXPECT_EQ(0, int32Result);
	EXPECT_EQ(OpcUa_Good, m_testee.toInt32(int32Result));
	EXPECT_EQ(0, int32Result);

	m_testee.setUInt32(numeric_limits<uint32_t>::max());
	EXPECT_EQ(OpcUa_BadOutOfRange, m_testee.toInt32(int32Result));

	m_testee.setUInt32(numeric_limits<int32_t>::max());
	EXPECT_EQ(OpcUa_Good, m_testee.toInt32(int32Result));
	EXPECT_EQ(numeric_limits<int32_t>::max(), int32Result);
}






