/* © Copyright Paris Moschovakos, CERN, 2026.  All rights not expressly granted are reserved.
 * uasdk_surface_test.cpp
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

#include <cstring>
#include "gtest/gtest.h"
#include <uavariant.h>
#include <uadatavalue.h>
#include <uadatetime.h>
#include <uadatavariablecache.h>
#include <opcua_basedatavariabletype.h>

TEST(UasdkSurfaceTest, changeTypeConvertsScalars)
{
	UaVariant v;
	v.setUInt32(1);
	EXPECT_EQ((OpcUa_StatusCode)OpcUa_Good, v.changeType(OpcUaType_Boolean, OpcUa_False));
	OpcUa_Boolean b = OpcUa_False;
	v.toBool(b);
	EXPECT_TRUE(b);

	v.setInt32(-7);
	EXPECT_EQ((OpcUa_StatusCode)OpcUa_Good, v.changeType(OpcUaType_Float, OpcUa_False));
	OpcUa_Float f = 0;
	EXPECT_EQ(OpcUa_Good, v.toFloat(f).statusCode());
	EXPECT_FLOAT_EQ(-7.0f, f);

	v.setDouble(3.5);
	EXPECT_EQ((OpcUa_StatusCode)OpcUa_Good, v.changeType(OpcUaType_String, OpcUa_False));
	EXPECT_EQ((OpcUa_StatusCode)OpcUa_BadNotSupported, v.changeType(OpcUaType_Int32, OpcUa_True));
}

TEST(UasdkSurfaceTest, dataValueDefaultCtorAndSetters)
{
	UaDataValue dv;
	UaVariant value;
	value.setInt16(42);
	dv.setValue(value, OpcUa_False);
	dv.setStatusCode(OpcUa_BadInternalError);
	UaDateTime stamp = UaDateTime::fromTime_t(1765000000);
	dv.setSourceTimestamp(stamp);
	dv.setServerTimestamp(stamp);
	EXPECT_EQ((OpcUa_StatusCode)OpcUa_BadInternalError, dv.statusCode());
	EXPECT_EQ(1765000000, dv.sourceTimestamp().toTime_t());
	OpcUa_Int16 out = 0;
	UaVariant(*dv.value()).toInt16(out);
	EXPECT_EQ(42, out);

	UaDataValue dv2;
	UaVariant value2;
	value2.setFloat(1.5f);
	dv2.setDataValue(value2, OpcUa_False, OpcUa_Good, stamp, stamp);
	EXPECT_EQ((OpcUa_StatusCode)OpcUa_Good, dv2.statusCode());
	EXPECT_EQ(1765000000, dv2.serverTimestamp().toTime_t());
}

TEST(UasdkSurfaceTest, dateTimeTimeTRoundTrip)
{
	time_t epoch = 1700000123;
	EXPECT_EQ(epoch, UaDateTime::fromTime_t(epoch).toTime_t());
}

TEST(UasdkSurfaceTest, qualifiedNameUasdkCtorAndEquality)
{
	UaQualifiedName uasdkOrder(UaString("address"), 2);
	EXPECT_EQ(2, uasdkOrder.namespaceIndex());
	EXPECT_TRUE(uasdkOrder == UaQualifiedName(2, UaString("address")));
	EXPECT_FALSE(uasdkOrder == UaQualifiedName(0, UaString("address")));
	EXPECT_FALSE(uasdkOrder == UaQualifiedName(2, UaString("other")));
}

TEST(UasdkSurfaceTest, stringUtf8IsCStringAndBangEquals)
{
	UaString s("hello");
	EXPECT_EQ(0, strcmp(s.toUtf8(), "hello"));
	EXPECT_TRUE(s != UaString("world"));
	EXPECT_FALSE(s != UaString("hello"));
	UaString joined = s + UaString(" there");
	EXPECT_EQ(0, strcmp(joined.toUtf8(), "hello there"));
}

TEST(UasdkSurfaceTest, referenceListsWalkByBrowseName)
{
	UaPropertyCache parent(UaString("parent"), UaNodeId(UaString("p"), 2), UaVariant(OpcUa_UInt32(1)), 0x1, UaString("en_US"));
	UaPropertyCache childA(UaString("alpha"), UaNodeId(UaString("p.alpha"), 2), UaVariant(OpcUa_UInt32(2)), 0x1, UaString("en_US"));
	UaPropertyCache childB(UaString("beta"), UaNodeId(UaString("p.beta"), 2), UaVariant(OpcUa_UInt32(3)), 0x1, UaString("en_US"));
	parent.addReferencedTarget(&childA, UaNodeId(47, 0));
	parent.addReferencedTarget(&childB, UaNodeId(47, 0));

	UaReferenceLists* lists = parent.getUaReferenceLists();
	ASSERT_NE(nullptr, lists);
	UaNode* hit = lists->getTargetNodeByBrowseName(UaQualifiedName(UaString("beta"), 2));
	ASSERT_NE(nullptr, hit);
	EXPECT_TRUE(hit->nodeId() == UaNodeId(UaString("p.beta"), 2));
	EXPECT_EQ(nullptr, lists->getTargetNodeByBrowseName(UaQualifiedName(UaString("gamma"), 2)));
	EXPECT_EQ(nullptr, lists->getTargetNodeByBrowseName(UaQualifiedName(UaString("beta"), 0)));
}

TEST(UasdkSurfaceTest, uaVariableDowncastAndSetValue)
{
	OpcUa::BaseDataVariableType variable(
			UaNodeId(UaString("var1"), 2),
			UaString("var1"),
			2,
			UaVariant(OpcUa_UInt32(0)),
			UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE,
			nullptr);
	UaNode* node = &variable;
	UaVariable* asVariable = (UaVariable*)node;
	UaDataValue dv(UaVariant(OpcUa_UInt32(123)), OpcUa_Good, UaDateTime::now(), UaDateTime::now());
	EXPECT_EQ(OpcUa_Good, asVariable->setValue(nullptr, dv, OpcUa_False).statusCode());
	OpcUa_UInt32 out = 0;
	UaVariant(*asVariable->value(nullptr).value()).toUInt32(out);
	EXPECT_EQ(123u, out);
}

#include <uathread.h>
#include <uasemaphore.h>
#include <uaobjecttypes.h>
#include <srvtrace.h>

namespace
{
class CountingThread: public UaThread
{
public:
	std::atomic_int counter{0};
protected:
	void run() override { ++counter; }
};
}

TEST(UasdkSurfaceTest, threadRunsAndJoins)
{
	CountingThread t;
	t.start();
	EXPECT_TRUE(t.wait());
	EXPECT_EQ(1, t.counter);
	EXPECT_TRUE(t.finished());
}

TEST(UasdkSurfaceTest, semaphoreWaitPostAndTimeout)
{
	UaSemaphore sem(0, 1);
	EXPECT_EQ((OpcUa_StatusCode)OpcUa_BadTimeout, sem.timedWait(20));
	EXPECT_EQ((OpcUa_StatusCode)OpcUa_Good, sem.post(1));
	EXPECT_EQ((OpcUa_StatusCode)OpcUa_Good, sem.wait());
	EXPECT_EQ((OpcUa_StatusCode)OpcUa_BadTimeout, sem.timedWait(20));
}
