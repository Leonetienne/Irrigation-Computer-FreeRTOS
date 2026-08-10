#include <catch2/catch_test_macros.hpp>
#include "test/stubs/MqttStub.h"

TEST_CASE("MqttStub", "[MqttStub]") {
    MqttStub stub;

    SECTION("default state is Disconnected") {
        REQUIRE(stub.getState() == MqttConnectionState::Disconnected);
    }

    SECTION("begin records the connect options") {
        stub.begin(MqttConnectOptions{"mqtt://broker", "user", "pass", "tele/LWT", "Offline"});
        REQUIRE(stub.getLastConnectOptions().brokerUri == "mqtt://broker");
        REQUIRE(stub.getLastConnectOptions().username == "user");
        REQUIRE(stub.getLastConnectOptions().password == "pass");
        REQUIRE(stub.getLastConnectOptions().lwtTopic == "tele/LWT");
        REQUIRE(stub.getLastConnectOptions().lwtMessage == "Offline");
    }

    SECTION("begin increments the call count") {
        stub.begin(MqttConnectOptions{});
        stub.begin(MqttConnectOptions{});
        REQUIRE(stub.getBeginCallCount() == 2);
    }

    SECTION("publish records the message") {
        stub.publish("stat/valve/0", "ON", 1, true);
        REQUIRE(stub.getPublishedMessages().size() == 1);
        REQUIRE(stub.getPublishedMessages()[0].topic == "stat/valve/0");
        REQUIRE(stub.getPublishedMessages()[0].payload == "ON");
        REQUIRE(stub.getPublishedMessages()[0].qos == 1);
        REQUIRE(stub.getPublishedMessages()[0].retain == true);
    }

    SECTION("subscribe records the topic") {
        stub.subscribe("cmnd/valve/0", 1);
        REQUIRE(stub.getSubscriptions().size() == 1);
        REQUIRE(stub.getSubscriptions()[0].topic == "cmnd/valve/0");
        REQUIRE(stub.getSubscriptions()[0].qos == 1);
    }

    SECTION("simulateConnected sets state to Connected") {
        stub.simulateConnected();
        REQUIRE(stub.getState() == MqttConnectionState::Connected);
    }

    SECTION("simulateConnected fires the onConnected callback") {
        bool called = false;
        stub.setOnConnected([&called]() { called = true; });
        stub.simulateConnected();
        REQUIRE(called);
    }

    SECTION("simulateDisconnected sets state to Disconnected") {
        stub.simulateConnected();
        stub.simulateDisconnected();
        REQUIRE(stub.getState() == MqttConnectionState::Disconnected);
    }

    SECTION("simulateDisconnected fires the onDisconnected callback") {
        bool called = false;
        stub.setOnDisconnected([&called]() { called = true; });
        stub.simulateDisconnected();
        REQUIRE(called);
    }

    SECTION("simulateMessage fires the onMessage callback with topic and payload") {
        std::string gotTopic;
        std::string gotPayload;
        stub.setOnMessage([&gotTopic, &gotPayload](const std::string& topic, const std::string& payload) {
            gotTopic = topic;
            gotPayload = payload;
        });
        stub.simulateMessage("cmnd/valve/0", "ON");
        REQUIRE(gotTopic == "cmnd/valve/0");
        REQUIRE(gotPayload == "ON");
    }

    SECTION("free resets state to Disconnected") {
        stub.simulateConnected();
        stub.free();
        REQUIRE(stub.getState() == MqttConnectionState::Disconnected);
    }
}
