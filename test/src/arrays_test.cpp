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

