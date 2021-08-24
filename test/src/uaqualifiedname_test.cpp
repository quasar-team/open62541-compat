/*
 * uaqualifiedname_test.cpp
 *
 *  Created on: 16 July 2021
 *      Author: ben-farnham 
 */

#include "gtest/gtest.h"
#include <other.h>

#include <other.h>

TEST(UaQualifiedNameTest, testToString)
{
    UaQualifiedName testee(2, "a.b.c");
    EXPECT_EQ("a.b.c", testee.toString().toUtf8());
}

TEST(UaQualifiedNameTest, testToFullString)
{
    UaQualifiedName testee(2, "a.b.c");
    EXPECT_EQ("ns=2|a.b.c", testee.toFullString().toUtf8());
}
