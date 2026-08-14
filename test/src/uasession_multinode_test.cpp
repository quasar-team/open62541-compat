/* © Copyright Paris Moschovakos, CERN, 2026.  All rights not expressly granted are reserved.
 * uasession_multinode_test.cpp
 *
 *  Created on: 14 Aug, 2026
 *      Author: Paris Moschovakos <paris.moschovakos@cern.ch>
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

#include <vector>

#include "gtest/gtest.h"

#include <LogIt.h>
#include <uaserver.h>
#include <nodemanagerbase.h>
#include <opcua_basedatavariabletype.h>
#include <opcua_identifiers.h>
#include <uaclient/uaclientsdk.h>

namespace
{

class MultinodeNodeManager: public NodeManagerBase
{
public:
    MultinodeNodeManager(): NodeManagerBase("urn:multinode:test") {}

    virtual UaStatus afterStartUp() override
    {
        addVariable("TestInt32", UaVariant((OpcUa_Int32)100));
        addVariable("TestDouble", UaVariant((OpcUa_Double)2.5));
        addVariable("TestBoolean", UaVariant((OpcUa_Boolean)OpcUa_False));
        addVariable("TestInt32b", UaVariant((OpcUa_Int32)200));
        return OpcUa_Good;
    }

private:
    void addVariable(const char* name, const UaVariant& initialValue)
    {
        OpcUa::BaseDataVariableType* variable = new OpcUa::BaseDataVariableType(
            UaNodeId(name, 2), name, 2, initialValue, 0x3, this);
        addNodeAndReference(UaNodeId(OpcUaId_ObjectsFolder, 0), variable, UaNodeId(OpcUaId_Organizes, 0));
    }
};

struct MultinodeServerFixture
{
    MultinodeServerFixture()
    {
        static bool logReady = [](){ Log::initializeLogging(Log::INF); return true; }();
        (void)logReady;
        runningFlag = OpcUa_True;
        nm = new MultinodeNodeManager();
        server = new UaServer();
        server->linkRunningFlag(&runningFlag);
        server->addNodeManager(nm);
        server->start();
    }

    ~MultinodeServerFixture()
    {
        runningFlag = OpcUa_False;
        server->stop();
        delete server;
    }

    volatile OpcUa_Boolean runningFlag;
    MultinodeNodeManager* nm;
    UaServer* server;
};

UaStatus connectSession(UaClientSdk::UaSession& session)
{
    UaClientSdk::SessionConnectInfo info;
    info.internalServiceCallTimeout = 2000;
    UaClientSdk::SessionSecurityInfo security;
    return session.connect("opc.tcp://127.0.0.1:4841", info, security, nullptr);
}

UaStatus readNodes(
    UaClientSdk::UaSession& session,
    const std::vector<UaNodeId>& nodes,
    UaDataValues& values)
{
    ServiceSettings settings;
    UaReadValueIds ids;
    ids.create(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i)
        nodes[i].copyTo(&ids[i].NodeId);
    UaDiagnosticInfos diagnosticInfos;
    return session.read(settings, 0, OpcUa_TimestampsToReturn_Both, ids, values, diagnosticInfos);
}

UaStatus writeNodes(
    UaClientSdk::UaSession& session,
    const std::vector<UaNodeId>& nodes,
    const std::vector<UaVariant>& newValues,
    UaStatusCodeArray& results)
{
    ServiceSettings settings;
    UaWriteValues writeValues;
    writeValues.create(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        nodes[i].copyTo(&writeValues[i].NodeId);
        writeValues[i].Value.Value = newValues[i];
    }
    UaDiagnosticInfos diagnosticInfos;
    return session.write(settings, writeValues, results, diagnosticInfos);
}

void disconnectSession(UaClientSdk::UaSession& session)
{
    ServiceSettings settings;
    session.disconnect(settings, OpcUa_True);
}

}

TEST(MultinodeSessionTest, singleNodeReadAndWriteRegression)
{
    MultinodeServerFixture fixture;
    UaClientSdk::UaSession session;
    ASSERT_TRUE(connectSession(session).isGood());

    UaDataValues values;
    ASSERT_TRUE(readNodes(session, {UaNodeId("TestInt32", 2)}, values).isGood());
    ASSERT_EQ(1u, values.size());
    EXPECT_TRUE(values[0].StatusCode.isGood());
    OpcUa_Int32 int32Value = 0;
    EXPECT_TRUE(UaVariant(values[0].Value).toInt32(int32Value).isGood());
    EXPECT_EQ(100, int32Value);

    UaStatusCodeArray results;
    ASSERT_TRUE(writeNodes(session, {UaNodeId("TestInt32b", 2)}, {UaVariant((OpcUa_Int32)7)}, results).isGood());
    ASSERT_EQ(1u, results.size());
    EXPECT_TRUE(UaStatus(results[0]).isGood());

    UaDataValues readBack;
    ASSERT_TRUE(readNodes(session, {UaNodeId("TestInt32b", 2)}, readBack).isGood());
    ASSERT_EQ(1u, readBack.size());
    int32Value = 0;
    EXPECT_TRUE(UaVariant(readBack[0].Value).toInt32(int32Value).isGood());
    EXPECT_EQ(7, int32Value);

    disconnectSession(session);
}

TEST(MultinodeSessionTest, batchReadThreeNodesDeliversAlignedValues)
{
    MultinodeServerFixture fixture;
    UaClientSdk::UaSession session;
    ASSERT_TRUE(connectSession(session).isGood());

    UaDataValues values;
    UaStatus status = readNodes(
        session,
        {UaNodeId("TestInt32", 2), UaNodeId("TestDouble", 2), UaNodeId("TestBoolean", 2)},
        values);
    ASSERT_TRUE(status.isGood());
    ASSERT_EQ(3u, values.size());

    for (size_t i = 0; i < values.size(); ++i)
    {
        EXPECT_TRUE(values[i].StatusCode.isGood());
        EXPECT_NE(0, static_cast<UA_DateTime>(values[i].SourceTimestamp));
        EXPECT_NE(0, static_cast<UA_DateTime>(values[i].ServerTimestamp));
    }

    OpcUa_Int32 int32Value = 0;
    EXPECT_TRUE(UaVariant(values[0].Value).toInt32(int32Value).isGood());
    EXPECT_EQ(100, int32Value);
    OpcUa_Double doubleValue = 0;
    EXPECT_TRUE(UaVariant(values[1].Value).toDouble(doubleValue).isGood());
    EXPECT_DOUBLE_EQ(2.5, doubleValue);
    OpcUa_Boolean booleanValue = OpcUa_True;
    EXPECT_TRUE(UaVariant(values[2].Value).toBool(booleanValue).isGood());
    EXPECT_FALSE(booleanValue);

    disconnectSession(session);
}

TEST(MultinodeSessionTest, batchWriteThreeNodesPersistsValues)
{
    MultinodeServerFixture fixture;
    UaClientSdk::UaSession session;
    ASSERT_TRUE(connectSession(session).isGood());

    std::vector<UaNodeId> nodes {
        UaNodeId("TestInt32", 2), UaNodeId("TestDouble", 2), UaNodeId("TestBoolean", 2)};
    UaStatusCodeArray results;
    UaStatus status = writeNodes(
        session,
        nodes,
        {UaVariant((OpcUa_Int32)42), UaVariant((OpcUa_Double)3.25), UaVariant((OpcUa_Boolean)OpcUa_True)},
        results);
    ASSERT_TRUE(status.isGood());
    ASSERT_EQ(3u, results.size());
    for (size_t i = 0; i < results.size(); ++i)
        EXPECT_TRUE(UaStatus(results[i]).isGood());

    UaDataValues readBack;
    ASSERT_TRUE(readNodes(session, nodes, readBack).isGood());
    ASSERT_EQ(3u, readBack.size());
    OpcUa_Int32 int32Value = 0;
    EXPECT_TRUE(UaVariant(readBack[0].Value).toInt32(int32Value).isGood());
    EXPECT_EQ(42, int32Value);
    OpcUa_Double doubleValue = 0;
    EXPECT_TRUE(UaVariant(readBack[1].Value).toDouble(doubleValue).isGood());
    EXPECT_DOUBLE_EQ(3.25, doubleValue);
    OpcUa_Boolean booleanValue = OpcUa_False;
    EXPECT_TRUE(UaVariant(readBack[2].Value).toBool(booleanValue).isGood());
    EXPECT_TRUE(booleanValue);

    disconnectSession(session);
}

TEST(MultinodeSessionTest, mixedBatchReportsBadNodeOnlyAtItsIndex)
{
    MultinodeServerFixture fixture;
    UaClientSdk::UaSession session;
    ASSERT_TRUE(connectSession(session).isGood());

    UaDataValues values;
    UaStatus status = readNodes(
        session,
        {UaNodeId("TestInt32", 2), UaNodeId("NoSuchNode", 2), UaNodeId("TestDouble", 2)},
        values);
    ASSERT_TRUE(status.isGood());
    ASSERT_EQ(3u, values.size());
    EXPECT_TRUE(values[0].StatusCode.isGood());
    EXPECT_EQ(OpcUa_BadNodeIdUnknown, values[1].StatusCode.statusCode());
    EXPECT_TRUE(values[2].StatusCode.isGood());
    OpcUa_Int32 int32Value = 0;
    EXPECT_TRUE(UaVariant(values[0].Value).toInt32(int32Value).isGood());
    EXPECT_EQ(100, int32Value);
    OpcUa_Double doubleValue = 0;
    EXPECT_TRUE(UaVariant(values[2].Value).toDouble(doubleValue).isGood());
    EXPECT_DOUBLE_EQ(2.5, doubleValue);

    UaStatusCodeArray results;
    status = writeNodes(
        session,
        {UaNodeId("TestInt32", 2), UaNodeId("NoSuchNode", 2), UaNodeId("TestDouble", 2)},
        {UaVariant((OpcUa_Int32)1000), UaVariant((OpcUa_Int32)5), UaVariant((OpcUa_Double)9.5)},
        results);
    ASSERT_TRUE(status.isGood());
    ASSERT_EQ(3u, results.size());
    EXPECT_TRUE(UaStatus(results[0]).isGood());
    EXPECT_EQ(OpcUa_BadNodeIdUnknown, results[1]);
    EXPECT_TRUE(UaStatus(results[2]).isGood());

    UaDataValues readBack;
    ASSERT_TRUE(readNodes(session, {UaNodeId("TestInt32", 2), UaNodeId("TestDouble", 2)}, readBack).isGood());
    ASSERT_EQ(2u, readBack.size());
    int32Value = 0;
    EXPECT_TRUE(UaVariant(readBack[0].Value).toInt32(int32Value).isGood());
    EXPECT_EQ(1000, int32Value);
    doubleValue = 0;
    EXPECT_TRUE(UaVariant(readBack[1].Value).toDouble(doubleValue).isGood());
    EXPECT_DOUBLE_EQ(9.5, doubleValue);

    disconnectSession(session);
}

TEST(MultinodeSessionTest, emptyBatchReturnsBadNothingToDoWithEmptyOutputs)
{
    MultinodeServerFixture fixture;
    UaClientSdk::UaSession session;
    ASSERT_TRUE(connectSession(session).isGood());

    UaDataValues values;
    values.create(5);
    UaStatus status = readNodes(session, {}, values);
    EXPECT_EQ(OpcUa_BadNothingToDo, status.statusCode());
    EXPECT_EQ(0u, values.size());

    UaStatusCodeArray results;
    results.create(4);
    status = writeNodes(session, {}, {}, results);
    EXPECT_EQ(OpcUa_BadNothingToDo, status.statusCode());
    EXPECT_EQ(0u, results.size());

    disconnectSession(session);
}

TEST(MultinodeSessionTest, badServiceStatusYieldsEmptyOutputs)
{
    UaClientSdk::UaSession session;
    {
        MultinodeServerFixture fixture;
        ASSERT_TRUE(connectSession(session).isGood());
    }

    UaDataValues values;
    values.create(3);
    UaStatus status = readNodes(
        session,
        {UaNodeId("TestInt32", 2), UaNodeId("TestDouble", 2), UaNodeId("TestBoolean", 2)},
        values);
    EXPECT_FALSE(status.isGood());
    EXPECT_EQ(0u, values.size());

    UaStatusCodeArray results;
    results.create(3);
    status = writeNodes(session, {UaNodeId("TestInt32", 2)}, {UaVariant((OpcUa_Int32)1)}, results);
    EXPECT_FALSE(status.isGood());
    EXPECT_EQ(0u, results.size());

    disconnectSession(session);
}

TEST(MultinodeSessionTest, presizedValuesArrayIsResizedByRead)
{
    MultinodeServerFixture fixture;
    UaClientSdk::UaSession session;
    ASSERT_TRUE(connectSession(session).isGood());

    UaDataValues values;
    values.create(10);
    UaStatus status = readNodes(
        session,
        {UaNodeId("TestInt32", 2), UaNodeId("TestDouble", 2), UaNodeId("TestBoolean", 2)},
        values);
    ASSERT_TRUE(status.isGood());
    ASSERT_EQ(3u, values.size());
    for (size_t i = 0; i < values.size(); ++i)
        EXPECT_TRUE(values[i].StatusCode.isGood());

    disconnectSession(session);
}

TEST(MultinodeSessionTest, largeBatchReadTwentySevenNodes)
{
    MultinodeServerFixture fixture;
    UaClientSdk::UaSession session;
    ASSERT_TRUE(connectSession(session).isGood());

    const std::vector<UaNodeId> cycle {
        UaNodeId("TestInt32", 2), UaNodeId("TestDouble", 2),
        UaNodeId("TestBoolean", 2), UaNodeId("TestInt32b", 2)};
    std::vector<UaNodeId> nodes;
    for (size_t i = 0; i < 27; ++i)
        nodes.push_back(cycle[i % cycle.size()]);

    UaDataValues values;
    UaStatus status = readNodes(session, nodes, values);
    ASSERT_TRUE(status.isGood());
    ASSERT_EQ(27u, values.size());
    for (size_t i = 0; i < values.size(); ++i)
    {
        EXPECT_TRUE(values[i].StatusCode.isGood());
        EXPECT_NE(0, static_cast<UA_DateTime>(values[i].ServerTimestamp));
    }

    disconnectSession(session);
}
