#include <catch2/catch_test_macros.hpp>
#include "test/stubs/WifiManagerStub.h"

TEST_CASE("WifiManagerStub", "[WifiManagerStub]") {
    WifiManagerStub stub;

    SECTION("default state is Disconnected") {
        REQUIRE(stub.getState() == WifiConnectionState::Disconnected);
    }

    SECTION("begin records ssid and password") {
        stub.begin("my_example_ap", "1234");
        REQUIRE(stub.getLastSsid() == "my_example_ap");
        REQUIRE(stub.getLastPassword() == "1234");
    }

    SECTION("begin increments the call count") {
        stub.begin("my_example_ap", "1234");
        stub.begin("my_example_ap", "1234");
        REQUIRE(stub.getBeginCallCount() == 2);
    }

    SECTION("simulateConnected sets state to Connected") {
        stub.simulateConnected();
        REQUIRE(stub.getState() == WifiConnectionState::Connected);
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
        REQUIRE(stub.getState() == WifiConnectionState::Disconnected);
    }

    SECTION("simulateDisconnected fires the onDisconnected callback") {
        bool called = false;
        stub.setOnDisconnected([&called]() { called = true; });
        stub.simulateDisconnected();
        REQUIRE(called);
    }

    SECTION("forceState sets the state directly") {
        stub.forceState(WifiConnectionState::Failed);
        REQUIRE(stub.getState() == WifiConnectionState::Failed);
    }
}