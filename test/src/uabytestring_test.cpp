/*
 * uabytestring_test.cpp
 *
 *  Created on: 27 Mar 2020
 *      Author: pnikiel
 */

#include "gtest/gtest.h"
#include <algorithm>

#include <uabytestring.h>
#include <simple_arrays.h>

const OpcUa_Byte reference[] = {1, 3, 5, 7, 9, 2, 4, 6, 8, 10};
const size_t referenceSize = sizeof reference / sizeof reference[0];

TEST(UaByteStringTest, testDefaultConstructor)
{
    UaByteString nullByteString;
    EXPECT_EQ(nullByteString.length(), 0);
    EXPECT_EQ(nullByteString.data(), nullptr);
}

TEST(UaByteStringTest, testFromDataConstructor)
{
    UaByteString byteString(referenceSize, reference);
    EXPECT_EQ(byteString.length(), referenceSize);
    bool equal = std::equal(
            std::begin(reference),
            std::end(reference),
            byteString.data());
   EXPECT_EQ(equal, true);
}

TEST(UaByteStringTest, testCopyConstructorUaByteString)
{
    UaByteString byteString(referenceSize, reference);
    UaByteString copiedByteString (byteString);
    EXPECT_EQ(copiedByteString.length(), referenceSize);
    bool equal = std::equal(
            std::begin(reference),
            std::end(reference),
            copiedByteString.data());
   EXPECT_EQ(equal, true);
}

TEST(UaByteStringTest, testCopyConstructorUA_ByteString)
{
    /* prepare stack ByteString */
    UA_ByteString stackByteString;
    UA_ByteString_init(&stackByteString);
    UA_ByteString_allocBuffer(&stackByteString, referenceSize);
    std::copy(
        std::begin(reference),
        std::end(reference),
        stackByteString.data);

    /* now construct open62541-compat ByteString from stack ByteString */
    UaByteString byteString (stackByteString);
    EXPECT_EQ(byteString.length(), referenceSize);
    bool equal = std::equal(
            std::begin(reference),
            std::end(reference),
            byteString.data());
    EXPECT_EQ(equal, true);
}

TEST(UaByteStringTest, testAssignmentOperator)
{
    UaByteString referenceByteString(referenceSize, reference);

    UaByteString another;
    another = referenceByteString;

    EXPECT_EQ(another.length(), referenceSize);
    bool equal = std::equal(
            std::begin(reference),
            std::end(reference),
            another.data());
    EXPECT_EQ(equal, true);
}

TEST(UaByteStringTest, testByteStringArray)
{
    const unsigned int N = 100000;
    UaByteStringArray array;
    array.create(N);
    for (unsigned int i=0; i<N; i++)
    {
        array[i] = UaByteString(referenceSize, reference);
    }
    // now test...
    for (unsigned int i=0; i<N; i++)
    {
        EXPECT_EQ(array[i].length(), referenceSize);
        bool equal = std::equal(
                std::begin(reference),
                std::end(reference),
                array[i].data());
        EXPECT_EQ(equal, true);
    }
}
