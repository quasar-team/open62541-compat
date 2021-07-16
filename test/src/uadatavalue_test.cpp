/*
 * uadatavalue_test.cpp
 *
 *  Created on: 16 July 2021
 *      Author: ben-farnham 
 */

#include "gtest/gtest.h"
#include <uadatavalue.h>

TEST(UaDataValueTest, testOperatorIsEqual)
{
    UaDataValue testee( UaVariant("some string value"), OpcUa_Good, UaDateTime::now(), UaDateTime::now() );
    EXPECT_TRUE(testee == testee);
}

