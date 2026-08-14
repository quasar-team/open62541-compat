/* © Copyright Paris Moschovakos, CERN, 2026.  All rights not expressly granted are reserved.
 * uavariant_matrix_test.cpp
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
#include "uadatavalue.h"
#include "uadatetime.h"

#include <open62541.h>

#include <stdexcept>

namespace
{

UaInt32Array makeInt32Array(std::initializer_list<OpcUa_Int32> values)
{
    UaInt32Array result;
    result.create(values.size());
    size_t i = 0;
    for (OpcUa_Int32 value : values)
        result[i++] = value;
    return result;
}

}

TEST_F(UaVariantTest, testInt32MatrixRoundTrip)
{
    UaInt32Array values = makeInt32Array({10, 11, 12, 13, 14, 15});
    UaInt32Array dimensions = makeInt32Array({2, 3});

    EXPECT_EQ(OpcUa_Good, m_testee.setInt32Matrix(values, dimensions));

    EXPECT_TRUE(m_testee.isMatrix());
    EXPECT_FALSE(m_testee.isArray());
    EXPECT_EQ(OpcUa_VariantArrayType_Matrix, m_testee.arrayType());
    EXPECT_EQ(6, m_testee.noOfMatrixElements());
    EXPECT_EQ(2, m_testee.dimensionSize());
    EXPECT_EQ(-1, m_testee.arraySize());

    UaUInt32Array reportedDimensions;
    m_testee.arrayDimensions(reportedDimensions);
    ASSERT_EQ(2u, reportedDimensions.size());
    EXPECT_EQ(2u, reportedDimensions[0]);
    EXPECT_EQ(3u, reportedDimensions[1]);

    UaInt32Array valuesOut;
    UaInt32Array dimensionsOut;
    ASSERT_EQ(OpcUa_Good, m_testee.toInt32Matrix(valuesOut, dimensionsOut));
    ASSERT_EQ(6u, valuesOut.size());
    for (size_t i = 0; i < 6; ++i)
        EXPECT_EQ(static_cast<OpcUa_Int32>(10 + i), valuesOut[i]);
    ASSERT_EQ(2u, dimensionsOut.size());
    EXPECT_EQ(2, dimensionsOut[0]);
    EXPECT_EQ(3, dimensionsOut[1]);
}

TEST_F(UaVariantTest, testDoubleMatrixRoundTrip)
{
    UaDoubleArray values;
    values.create(4);
    for (size_t i = 0; i < 4; ++i)
        values[i] = 0.5 * i;
    UaInt32Array dimensions = makeInt32Array({2, 2});

    EXPECT_EQ(OpcUa_Good, m_testee.setDoubleMatrix(values, dimensions));
    EXPECT_TRUE(m_testee.isMatrix());
    EXPECT_EQ(4, m_testee.noOfMatrixElements());

    UaDoubleArray valuesOut;
    UaInt32Array dimensionsOut;
    ASSERT_EQ(OpcUa_Good, m_testee.toDoubleMatrix(valuesOut, dimensionsOut));
    ASSERT_EQ(4u, valuesOut.size());
    for (size_t i = 0; i < 4; ++i)
        EXPECT_DOUBLE_EQ(0.5 * i, valuesOut[i]);
    ASSERT_EQ(2u, dimensionsOut.size());
    EXPECT_EQ(2, dimensionsOut[0]);
    EXPECT_EQ(2, dimensionsOut[1]);
}

TEST_F(UaVariantTest, testBooleanMatrixUasdkSpellings)
{
    UaBooleanArray values;
    values.create(4);
    values[0] = true;
    values[3] = true;
    UaInt32Array dimensions = makeInt32Array({2, 2});

    EXPECT_EQ(OpcUa_Good, m_testee.setBooleanMatrix(values, dimensions));
    EXPECT_TRUE(m_testee.isMatrix());

    UaBooleanArray valuesOut;
    UaInt32Array dimensionsOut;
    ASSERT_EQ(OpcUa_Good, m_testee.toBooleanMatrix(valuesOut, dimensionsOut));
    ASSERT_EQ(4u, valuesOut.size());
    EXPECT_TRUE(valuesOut[0]);
    EXPECT_FALSE(valuesOut[1]);
    EXPECT_FALSE(valuesOut[2]);
    EXPECT_TRUE(valuesOut[3]);
    ASSERT_EQ(OpcUa_Good, m_testee.toBoolMatrix(valuesOut, dimensionsOut));
}

TEST_F(UaVariantTest, testByteMatrixRoundTrip)
{
    UaByteArray values;
    values.create(6);
    for (size_t i = 0; i < 6; ++i)
        values[i] = static_cast<OpcUa_Byte>(0xA0 + i);
    UaInt32Array dimensions = makeInt32Array({3, 2});

    EXPECT_EQ(OpcUa_Good, m_testee.setByteMatrix(values, dimensions));

    UaByteArray valuesOut;
    UaInt32Array dimensionsOut;
    ASSERT_EQ(OpcUa_Good, m_testee.toByteMatrix(valuesOut, dimensionsOut));
    ASSERT_EQ(6u, valuesOut.size());
    for (size_t i = 0; i < 6; ++i)
        EXPECT_EQ(static_cast<OpcUa_Byte>(0xA0 + i), valuesOut[i]);
    ASSERT_EQ(2u, dimensionsOut.size());
    EXPECT_EQ(3, dimensionsOut[0]);
    EXPECT_EQ(2, dimensionsOut[1]);
}

TEST_F(UaVariantTest, testStringMatrixRoundTrip)
{
    UaStringArray values;
    values.create(4);
    values[0] = UaString("a");
    values[1] = UaString("bb");
    values[2] = UaString("ccc");
    values[3] = UaString("dddd");
    UaInt32Array dimensions = makeInt32Array({2, 2});

    EXPECT_EQ(OpcUa_Good, m_testee.setStringMatrix(values, dimensions));
    EXPECT_TRUE(m_testee.isMatrix());

    UaStringArray valuesOut;
    UaInt32Array dimensionsOut;
    ASSERT_EQ(OpcUa_Good, m_testee.toStringMatrix(valuesOut, dimensionsOut));
    ASSERT_EQ(4u, valuesOut.size());
    EXPECT_STREQ("a", valuesOut[0].toUtf8());
    EXPECT_STREQ("bb", valuesOut[1].toUtf8());
    EXPECT_STREQ("ccc", valuesOut[2].toUtf8());
    EXPECT_STREQ("dddd", valuesOut[3].toUtf8());
    ASSERT_EQ(2u, dimensionsOut.size());
}

TEST_F(UaVariantTest, testThreeDimensionalMatrix)
{
    UaInt32Array values;
    values.create(24);
    for (size_t i = 0; i < 24; ++i)
        values[i] = static_cast<OpcUa_Int32>(i);
    UaInt32Array dimensions = makeInt32Array({2, 3, 4});

    EXPECT_EQ(OpcUa_Good, m_testee.setInt32Matrix(values, dimensions));
    EXPECT_EQ(3, m_testee.dimensionSize());
    EXPECT_EQ(24, m_testee.noOfMatrixElements());

    UaUInt32Array reportedDimensions;
    m_testee.arrayDimensions(reportedDimensions);
    ASSERT_EQ(3u, reportedDimensions.size());
    EXPECT_EQ(2u, reportedDimensions[0]);
    EXPECT_EQ(3u, reportedDimensions[1]);
    EXPECT_EQ(4u, reportedDimensions[2]);
}

TEST_F(UaVariantTest, testMatrixDimensionsValidation)
{
    m_testee.setInt32(42);

    UaInt32Array values = makeInt32Array({1, 2, 3, 4, 5});
    UaInt32Array wrongDimensions = makeInt32Array({2, 3});
    EXPECT_EQ(OpcUa_BadInvalidArgument, m_testee.setInt32Matrix(values, wrongDimensions));

    UaInt32Array negativeDimensions = makeInt32Array({-2, 3});
    EXPECT_EQ(OpcUa_BadInvalidArgument, m_testee.setInt32Matrix(values, negativeDimensions));

    OpcUa_Int32 preservedValue = 0;
    EXPECT_EQ(OpcUa_Good, m_testee.toInt32(preservedValue));
    EXPECT_EQ(42, preservedValue);
    EXPECT_FALSE(m_testee.isMatrix());
}

TEST_F(UaVariantTest, testToMatrixTypeMismatch)
{
    UaInt32Array valuesOut;
    UaInt32Array dimensionsOut;

    EXPECT_EQ(OpcUa_BadTypeMismatch, m_testee.toInt32Matrix(valuesOut, dimensionsOut));

    m_testee.setInt32(42);
    EXPECT_EQ(OpcUa_BadTypeMismatch, m_testee.toInt32Matrix(valuesOut, dimensionsOut));

    UaInt32Array values = makeInt32Array({1, 2, 3});
    m_testee.setInt32Array(values);
    EXPECT_EQ(OpcUa_BadTypeMismatch, m_testee.toInt32Matrix(valuesOut, dimensionsOut));

    UaInt32Array dimensions = makeInt32Array({1, 3});
    EXPECT_EQ(OpcUa_Good, m_testee.setInt32Matrix(values, dimensions));
    UaDoubleArray doubleValuesOut;
    EXPECT_EQ(OpcUa_BadTypeMismatch, m_testee.toDoubleMatrix(doubleValuesOut, dimensionsOut));
}

TEST_F(UaVariantTest, testMatrixDetachNotImplementedThrows)
{
    UaInt32Array values = makeInt32Array({1, 2, 3, 4});
    UaInt32Array dimensions = makeInt32Array({2, 2});
    EXPECT_THROW(m_testee.setInt32Matrix(values, dimensions, OpcUa_True), std::runtime_error);
}

TEST_F(UaVariantTest, testOneDimensionalArrayBehaviourUnchanged)
{
    UaInt32Array values = makeInt32Array({7, 8, 9});
    m_testee.setInt32Array(values);

    EXPECT_TRUE(m_testee.isArray());
    EXPECT_FALSE(m_testee.isMatrix());
    EXPECT_EQ(OpcUa_VariantArrayType_Array, m_testee.arrayType());
    EXPECT_EQ(3, m_testee.arraySize());
    EXPECT_EQ(1, m_testee.dimensionSize());
    EXPECT_EQ(-1, m_testee.noOfMatrixElements());

    UaUInt32Array reportedDimensions;
    m_testee.arrayDimensions(reportedDimensions);
    ASSERT_EQ(1u, reportedDimensions.size());
    EXPECT_EQ(3u, reportedDimensions[0]);

    UaInt32Array valuesOut;
    ASSERT_EQ(OpcUa_Good, m_testee.toInt32Array(valuesOut));
    ASSERT_EQ(3u, valuesOut.size());
    EXPECT_EQ(7, valuesOut[0]);
    EXPECT_EQ(8, valuesOut[1]);
    EXPECT_EQ(9, valuesOut[2]);
}

TEST_F(UaVariantTest, testScalarBehaviourUnchanged)
{
    m_testee.setInt32(42);

    EXPECT_FALSE(m_testee.isArray());
    EXPECT_FALSE(m_testee.isMatrix());
    EXPECT_EQ(OpcUa_VariantArrayType_Scalar, m_testee.arrayType());
    EXPECT_EQ(-1, m_testee.arraySize());
    EXPECT_EQ(-1, m_testee.dimensionSize());
    EXPECT_EQ(-1, m_testee.noOfMatrixElements());

    UaUInt32Array reportedDimensions;
    m_testee.arrayDimensions(reportedDimensions);
    EXPECT_EQ(0u, reportedDimensions.size());
}

TEST_F(UaVariantTest, testMatrixCopyAndAssignOwnership)
{
    UaInt32Array values = makeInt32Array({10, 11, 12, 13, 14, 15});
    UaInt32Array dimensions = makeInt32Array({2, 3});
    ASSERT_EQ(OpcUa_Good, m_testee.setInt32Matrix(values, dimensions));

    UaVariant copied(m_testee);
    UaVariant assigned;
    assigned = m_testee;

    m_testee.clear();
    EXPECT_TRUE(m_testee.isEmpty());
    EXPECT_FALSE(m_testee.isMatrix());

    for (UaVariant* variant : {&copied, &assigned})
    {
        EXPECT_TRUE(variant->isMatrix());
        UaInt32Array valuesOut;
        UaInt32Array dimensionsOut;
        ASSERT_EQ(OpcUa_Good, variant->toInt32Matrix(valuesOut, dimensionsOut));
        ASSERT_EQ(6u, valuesOut.size());
        for (size_t i = 0; i < 6; ++i)
            EXPECT_EQ(static_cast<OpcUa_Int32>(10 + i), valuesOut[i]);
        ASSERT_EQ(2u, dimensionsOut.size());
        EXPECT_EQ(2, dimensionsOut[0]);
        EXPECT_EQ(3, dimensionsOut[1]);
    }

    EXPECT_TRUE(copied == assigned);
}

TEST_F(UaVariantTest, testMatrixEqualityIsRankSensitive)
{
    UaInt32Array values = makeInt32Array({1, 2, 3, 4, 5, 6});
    UaInt32Array dimensions23 = makeInt32Array({2, 3});
    UaInt32Array dimensions32 = makeInt32Array({3, 2});

    UaVariant matrix23;
    UaVariant matrix32;
    UaVariant flatArray;
    ASSERT_EQ(OpcUa_Good, matrix23.setInt32Matrix(values, dimensions23));
    ASSERT_EQ(OpcUa_Good, matrix32.setInt32Matrix(values, dimensions32));
    flatArray.setInt32Array(values);

    EXPECT_FALSE(matrix23 == matrix32);
    EXPECT_FALSE(matrix23 == flatArray);
    EXPECT_TRUE(matrix23 == UaVariant(matrix23));
}

TEST_F(UaVariantTest, testMatrixSurvivesUaDataValue)
{
    UaInt32Array values = makeInt32Array({10, 11, 12, 13, 14, 15});
    UaInt32Array dimensions = makeInt32Array({2, 3});
    ASSERT_EQ(OpcUa_Good, m_testee.setInt32Matrix(values, dimensions));

    UaDataValue dataValue(m_testee, OpcUa_Good, UaDateTime::now(), UaDateTime::now());
    UaVariant fromDataValue(*dataValue.value());

    EXPECT_TRUE(fromDataValue.isMatrix());
    UaInt32Array valuesOut;
    UaInt32Array dimensionsOut;
    ASSERT_EQ(OpcUa_Good, fromDataValue.toInt32Matrix(valuesOut, dimensionsOut));
    ASSERT_EQ(6u, valuesOut.size());
    ASSERT_EQ(2u, dimensionsOut.size());
    EXPECT_EQ(2, dimensionsOut[0]);
    EXPECT_EQ(3, dimensionsOut[1]);
}

TEST_F(UaVariantTest, testMatrixClear)
{
    UaInt32Array values = makeInt32Array({1, 2, 3, 4});
    UaInt32Array dimensions = makeInt32Array({2, 2});
    ASSERT_EQ(OpcUa_Good, m_testee.setInt32Matrix(values, dimensions));
    ASSERT_TRUE(m_testee.isMatrix());

    m_testee.clear();

    EXPECT_TRUE(m_testee.isEmpty());
    EXPECT_FALSE(m_testee.isMatrix());
    EXPECT_EQ(-1, m_testee.noOfMatrixElements());

    ASSERT_EQ(OpcUa_Good, m_testee.setInt32Matrix(values, dimensions));
    EXPECT_TRUE(m_testee.isMatrix());
    EXPECT_EQ(4, m_testee.noOfMatrixElements());
}

TEST_F(UaVariantTest, testNullMatrixDimensionsRejected)
{
    UaInt32Array empty;
    empty.create(0);

    UaInt32Array nullDimensions = makeInt32Array({-1, -1});
    EXPECT_EQ(OpcUa_BadInvalidArgument, m_testee.setInt32Matrix(empty, nullDimensions));

    UaInt32Array zeroDimensions = makeInt32Array({0, 0});
    EXPECT_EQ(OpcUa_BadInvalidArgument, m_testee.setInt32Matrix(empty, zeroDimensions));

    EXPECT_TRUE(m_testee.isEmpty());
    EXPECT_FALSE(m_testee.isMatrix());
}

TEST_F(UaVariantTest, testScalarSetOverMatrixReplacesValue)
{
    UaInt32Array values = makeInt32Array({1, 2, 3, 4, 5, 6});
    UaInt32Array dimensions = makeInt32Array({2, 3});
    ASSERT_EQ(OpcUa_Good, m_testee.setInt32Matrix(values, dimensions));

    m_testee.setInt32(42);

    EXPECT_FALSE(m_testee.isMatrix());
    EXPECT_FALSE(m_testee.isArray());
    EXPECT_EQ(OpcUa_VariantArrayType_Scalar, m_testee.arrayType());
    EXPECT_EQ(-1, m_testee.dimensionSize());
    OpcUa_Int32 scalarOut = 0;
    EXPECT_EQ(OpcUa_Good, m_testee.toInt32(scalarOut));
    EXPECT_EQ(42, scalarOut);
}

TEST_F(UaVariantTest, testMatrixOverMatrixReset)
{
    UaInt32Array values = makeInt32Array({1, 2, 3, 4, 5, 6});
    UaInt32Array dimensions23 = makeInt32Array({2, 3});
    ASSERT_EQ(OpcUa_Good, m_testee.setInt32Matrix(values, dimensions23));

    UaInt32Array smallerValues = makeInt32Array({7, 8, 9, 10});
    UaInt32Array dimensions22 = makeInt32Array({2, 2});
    ASSERT_EQ(OpcUa_Good, m_testee.setInt32Matrix(smallerValues, dimensions22));

    EXPECT_TRUE(m_testee.isMatrix());
    EXPECT_EQ(4, m_testee.noOfMatrixElements());
    UaInt32Array valuesOut;
    UaInt32Array dimensionsOut;
    ASSERT_EQ(OpcUa_Good, m_testee.toInt32Matrix(valuesOut, dimensionsOut));
    ASSERT_EQ(4u, valuesOut.size());
    EXPECT_EQ(10, valuesOut[3]);
    ASSERT_EQ(2u, dimensionsOut.size());
    EXPECT_EQ(2, dimensionsOut[0]);
    EXPECT_EQ(2, dimensionsOut[1]);
}

TEST_F(UaVariantTest, testMatrixSelfAssignment)
{
    UaInt32Array values = makeInt32Array({10, 11, 12, 13, 14, 15});
    UaInt32Array dimensions = makeInt32Array({2, 3});
    ASSERT_EQ(OpcUa_Good, m_testee.setInt32Matrix(values, dimensions));

    UaVariant& alias = m_testee;
    m_testee = alias;

    EXPECT_TRUE(m_testee.isMatrix());
    UaInt32Array valuesOut;
    UaInt32Array dimensionsOut;
    ASSERT_EQ(OpcUa_Good, m_testee.toInt32Matrix(valuesOut, dimensionsOut));
    ASSERT_EQ(6u, valuesOut.size());
    EXPECT_EQ(15, valuesOut[5]);
    ASSERT_EQ(2u, dimensionsOut.size());
}

TEST_F(UaVariantTest, testToMatrixInconsistentDimensionsRejected)
{
    UA_Variant raw;
    UA_Variant_init(&raw);
    UA_Int32* data = static_cast<UA_Int32*>(UA_Array_new(2, &UA_TYPES[UA_TYPES_INT32]));
    ASSERT_NE(nullptr, data);
    data[0] = 1;
    data[1] = 2;
    UA_Variant_setArray(&raw, data, 2, &UA_TYPES[UA_TYPES_INT32]);
    raw.arrayDimensions = static_cast<UA_UInt32*>(UA_Array_new(2, &UA_TYPES[UA_TYPES_UINT32]));
    ASSERT_NE(nullptr, raw.arrayDimensions);
    raw.arrayDimensionsSize = 2;
    raw.arrayDimensions[0] = 2;
    raw.arrayDimensions[1] = 3;

    UaVariant fromRaw(raw);
    UA_Variant_clear(&raw);

    UaInt32Array valuesOut;
    UaInt32Array dimensionsOut;
    EXPECT_EQ(OpcUa_BadInvalidArgument, fromRaw.toInt32Matrix(valuesOut, dimensionsOut));
    EXPECT_EQ(-1, fromRaw.noOfMatrixElements());
}

TEST_F(UaVariantTest, testEmptyVariantMatrixAccessors)
{
    EXPECT_TRUE(m_testee.isEmpty());
    EXPECT_FALSE(m_testee.isMatrix());
    EXPECT_EQ(OpcUa_VariantArrayType_Scalar, m_testee.arrayType());
    EXPECT_EQ(-1, m_testee.dimensionSize());
    EXPECT_EQ(-1, m_testee.noOfMatrixElements());
}

TEST_F(UaVariantTest, testUaBoolArrayAlias)
{
    UaBoolArray values;
    values.create(2);
    values[0] = true;
    UaInt32Array dimensions = makeInt32Array({2, 1});
    EXPECT_EQ(OpcUa_Good, m_testee.setBoolMatrix(values, dimensions));

    UaBoolArray valuesOut;
    UaInt32Array dimensionsOut;
    ASSERT_EQ(OpcUa_Good, m_testee.toBoolMatrix(valuesOut, dimensionsOut));
    ASSERT_EQ(2u, valuesOut.size());
    EXPECT_TRUE(valuesOut[0]);
    EXPECT_FALSE(valuesOut[1]);
}
