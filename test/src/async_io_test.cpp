/* © Copyright Paris Moschovakos, CERN, 2026.  All rights not expressly granted are reserved.
 * async_io_test.cpp
 *
 *  Created on: 9 Jun, 2026
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

#include <chrono>
#include <thread>

#include "gtest/gtest.h"

#include <LogIt.h>
#include <uaserver.h>
#include <nodemanagerbase.h>
#include <opcua_basedatavariabletype.h>
#include <opcua_identifiers.h>
#include <uaclient/uaclientsdk.h>
#include <uaplatformlayer.h>

namespace
{

class DeferringVariable: public OpcUa::BaseDataVariableType
{
public:
    DeferringVariable(
            const UaNodeId& nodeId,
            const UaString& name,
            OpcUa_UInt16 ns,
            const UaVariant& initialValue,
            NodeManagerConfig* nm,
            unsigned int delayMs):
        OpcUa::BaseDataVariableType(nodeId, name, ns, initialValue, 0x1, nm),
        m_delayMs(delayMs)
    {}

    virtual OpcUa_Boolean handlesIo() const override { return OpcUa_True; }

    virtual void beginRead (AsyncReadHandle handle) override
    {
        unsigned int delayMs = m_delayMs;
        std::shared_ptr<AsyncReadHandle> sharedHandle(new AsyncReadHandle(std::move(handle)));
        std::thread([sharedHandle, delayMs](){
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            UaVariant v;
            v.setDouble(3.14);
            sharedHandle->complete(UaDataValue(v, OpcUa_Good, UaDateTime::now(), UaDateTime::now()));
        }).detach();
    }

private:
    unsigned int m_delayMs;
};

class TestNodeManager: public NodeManagerBase
{
public:
    TestNodeManager(): NodeManagerBase("urn:asyncio:test"), slow(nullptr), fast(nullptr) {}

    virtual UaStatus afterStartUp() override
    {
        slow = new DeferringVariable(
            UaNodeId("slow", 2), "slow", 2, UaVariant(0.0), this, 1500);
        addNodeAndReference(UaNodeId(OpcUaId_ObjectsFolder, 0), slow, UaNodeId(OpcUaId_Organizes, 0));
        fast = new OpcUa::BaseDataVariableType(
            UaNodeId("fast", 2), "fast", 2, UaVariant((OpcUa_Int32)42), 0x1, this);
        addNodeAndReference(UaNodeId(OpcUaId_ObjectsFolder, 0), fast, UaNodeId(OpcUaId_Organizes, 0));
        return OpcUa_Good;
    }

    DeferringVariable* slow;
    OpcUa::BaseDataVariableType* fast;
};

struct TestServerFixture
{
    TestServerFixture()
    {
        static bool logReady = [](){ Log::initializeLogging(Log::INF); return true; }();
        (void)logReady;
        runningFlag = OpcUa_True;
        nm = new TestNodeManager();
        server = new UaServer();
        server->linkRunningFlag(&runningFlag);
        server->addNodeManager(nm);
        server->start();
    }

    ~TestServerFixture()
    {
        runningFlag = OpcUa_False;
        server->stop();
        delete server;
    }

    volatile OpcUa_Boolean runningFlag;
    TestNodeManager* nm;
    UaServer* server;
};

UaStatus readNode(UaClientSdk::UaSession& session, const UaNodeId& node, UaDataValues& out)
{
    ServiceSettings settings;
    UaReadValueIds ids;
    ids.create(1);
    node.copyTo(&ids[0].NodeId);
    out.create(1);
    UaDiagnosticInfos diag;
    return session.read(settings, 0, OpcUa_TimestampsToReturn_Both, ids, out, diag);
}

}

TEST(AsyncIoTest, deferredReadDeliversValueAndDoesNotBlockServer)
{
    TestServerFixture fixture;

    UaClientSdk::UaSession slowSession;
    UaClientSdk::UaSession fastSession;
    UaClientSdk::SessionConnectInfo info;
    info.internalServiceCallTimeout = 10000;
    UaClientSdk::SessionSecurityInfo security;
    ASSERT_TRUE(slowSession.connect("opc.tcp://127.0.0.1:4841", info, security, nullptr).isGood());
    ASSERT_TRUE(fastSession.connect("opc.tcp://127.0.0.1:4841", info, security, nullptr).isGood());

    std::atomic<bool> slowDone(false);
    UaStatus slowStatus;
    UaDataValues slowValues;
    std::thread slowReader([&](){
        slowStatus = readNode(slowSession, UaNodeId("slow", 2), slowValues);
        slowDone = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(slowDone.load());

    auto t0 = std::chrono::steady_clock::now();
    UaDataValues fastValues;
    UaStatus fastStatus = readNode(fastSession, UaNodeId("fast", 2), fastValues);
    double fastSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

    EXPECT_TRUE(fastStatus.isGood());
    EXPECT_LT(fastSeconds, 0.5) << "fast read must not be blocked by the in-flight deferred read";
    OpcUa_Int32 fastValue = 0;
    EXPECT_TRUE(UaVariant(fastValues[0].Value).toInt32(fastValue).isGood());
    EXPECT_EQ(42, fastValue);

    slowReader.join();
    EXPECT_TRUE(slowStatus.isGood());
    OpcUa_Double slowValue = 0;
    EXPECT_TRUE(UaVariant(slowValues[0].Value).toDouble(slowValue).isGood());
    EXPECT_DOUBLE_EQ(3.14, slowValue);

    ServiceSettings settings;
    slowSession.disconnect(settings, OpcUa_True);
    fastSession.disconnect(settings, OpcUa_True);
}

TEST(AsyncIoTest, inlineCompletionStaysSynchronous)
{
    TestServerFixture fixture;

    UaClientSdk::UaSession session;
    UaClientSdk::SessionConnectInfo info;
    UaClientSdk::SessionSecurityInfo security;
    ASSERT_TRUE(session.connect("opc.tcp://127.0.0.1:4841", info, security, nullptr).isGood());

    UaDataValues values;
    UaStatus status = readNode(session, UaNodeId("fast", 2), values);
    EXPECT_TRUE(status.isGood());
    OpcUa_Int32 value = 0;
    EXPECT_TRUE(UaVariant(values[0].Value).toInt32(value).isGood());
    EXPECT_EQ(42, value);

    ServiceSettings settings;
    session.disconnect(settings, OpcUa_True);
}
