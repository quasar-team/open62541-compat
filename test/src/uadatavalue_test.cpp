/*
 * uadatavalue_test.cpp
 *
 *  Created on: 16 July 2021
 *      Author: ben-farnham 
 */

#include "gtest/gtest.h"
#include <uadatavalue.h>

const auto g_sValueVariant = UaVariant(UaString("some string value"));
const auto g_sSourceTime = UaDateTime::fromString(UaString("2021-08-02T14:30:00Z"));
const auto g_sServerTime = UaDateTime::fromString(UaString("2021-08-02T14:30:01Z"));

TEST(UaDataValueTest, testOperatorIsEqual)
{
    UaDataValue testee(g_sValueVariant , OpcUa_Good, g_sServerTime, g_sSourceTime );
    EXPECT_TRUE(testee == testee);
    EXPECT_TRUE(testee == UaDataValue( g_sValueVariant, OpcUa_Good, g_sServerTime, g_sSourceTime ) ) << "equivalent object";
    EXPECT_FALSE(testee == UaDataValue( UaVariant(UaString("DIFFERENT string value")), OpcUa_Good, g_sServerTime, g_sSourceTime ) ) << "value changed";
    EXPECT_FALSE(testee == UaDataValue( UaVariant(OpcUa_Int32(69)), OpcUa_Good, g_sServerTime, g_sSourceTime ) ) << "type changed";
    EXPECT_FALSE(testee == UaDataValue( g_sValueVariant, OpcUa_Bad, g_sServerTime, g_sSourceTime ) ) << "status changed";
    EXPECT_FALSE(testee == UaDataValue( g_sValueVariant, OpcUa_Good, UaDateTime::fromString("2222-08-02T14:30:01Z"), g_sSourceTime ) ) << "server timestamp changed";
    EXPECT_FALSE(testee == UaDataValue( g_sValueVariant, OpcUa_Good, g_sServerTime, UaDateTime::fromString("2222-08-02T14:30:01Z") ) )<< "source timestamp changed";
}

TEST(UaDataValueTest, testOperatorNotIsEqual)
{
    UaDataValue testee( UaVariant("some string value"), OpcUa_Good, g_sServerTime, g_sSourceTime );
    EXPECT_FALSE(testee != testee);
}
