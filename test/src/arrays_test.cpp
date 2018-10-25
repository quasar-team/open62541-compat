/* © Copyright Ben Farnham, CERN, 2018.  All rights not expressly granted are reserved.
 * nodemanagerbase.cpp
 *
 *  Created on: reated on: Apr 11, 2018
 *      Author: Ben Farnham <ben.farnham@cern.ch>
 *      Author: Piotr Nikiel <piotr.nikiel@cern.ch>
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
#include "arrays.h"
#include "uavariant.h"

TEST(ArraysTest, testDefaultInitializer)
{
	UaVariantArray testee;
	EXPECT_EQ(0, testee.size()) << "default initialized array should be empty";
}

TEST(ArraysTest, testInitializeAndSetValues)
{
	UaVariantArray testee;

	testee.create(1000);
	for(size_t i=0; i<1000; ++i)
	{
		testee[i] = UaVariant(i);
	}

	EXPECT_EQ(1000, testee.size()) << "should be 1000 array elements";
	for(size_t i=0; i<1000; ++i)
	{
		EXPECT_EQ(UaVariant(i), testee[i]) << "element values should match index";
	}
}

TEST(ArraysTest, testUaVariantsHoldingArrays)
{
	UaInt32Array intArray;
	intArray.create(100);

	for(int32_t i=0; i<100; ++i)
	{
		intArray[i]=i;
	}

	UaVariant testee;

	testee.setInt32Array(intArray);
	EXPECT_TRUE(testee.isArray()) << "variant should know it now holds an array value";

	UaUInt32Array arrayDimensions;
	testee.arrayDimensions(arrayDimensions);

	EXPECT_EQ(1, arrayDimensions.size()) << "should be a 1D array";
	EXPECT_EQ(100, arrayDimensions[0]) << "should be 100 elements in the 1D";
}

TEST(ArraysTest, testEmptyArraySetter)
{
    UaInt32Array emptyArray;
    UaVariant testee;

    testee.setInt32Array(emptyArray);

    EXPECT_TRUE(testee.isArray()) << "it should be just an empty array";

    UaUInt32Array dimensions;
    testee.arrayDimensions(dimensions);
    EXPECT_EQ(0, dimensions[0]) << "arrayDimensions should say it's an empty array";
}

