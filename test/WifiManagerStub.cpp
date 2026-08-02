#include <catch2/catch_test_macros.hpp>
#include "test/stubs/WifiManagerStub.h"

TEST_CASE("WifiManagerStub", "[WifiManagerStub]") {
    WifiManagerStub stub;

    SECTION("default state is Disconnected") {
        REQUIRE(stub.getState() == WifiConnectionState::Disconnected);
    }

    SECTION("beginUserWifi records ssid and password") {
        stub.beginUserWifi(WifiCredentials{"my_example_ap", "1234"});
        REQUIRE(stub.getLastSsid() == "my_example_ap");
        REQUIRE(stub.getLastPassword() == "1234");
    }

    SECTION("beginUserWifi increments the call count") {
        stub.beginUserWifi(WifiCredentials{"my_example_ap", "1234"});
        stub.beginUserWifi(WifiCredentials{"my_example_ap", "1234"});
        REQUIRE(stub.getBeginUserWifiCallCount() == 2);
    }

    SECTION("beginOnboardingWifi increments its own call count") {
        stub.beginOnboardingWifi();
        stub.beginOnboardingWifi();
        REQUIRE(stub.getBeginOnboardingWifiCallCount() == 2);
    }

    SECTION("beginOnboardingWifi does not affect beginUserWifi's call count") {
        stub.beginOnboardingWifi();
        REQUIRE(stub.getBeginUserWifiCallCount() == 0);
    }

    SECTION("beginUserWifi does not affect beginOnboardingWifi's call count") {
        stub.beginUserWifi(WifiCredentials{"my_example_ap", "1234"});
        REQUIRE(stub.getBeginOnboardingWifiCallCount() == 0);
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

    SECTION("simulateFailed sets state to Failed") {
        stub.simulateFailed();
        REQUIRE(stub.getState() == WifiConnectionState::Failed);
    }

    SECTION("simulateFailed fires the onFailed callback") {
        bool called = false;
        stub.setOnFailed([&called]() { called = true; });
        stub.simulateFailed();
        REQUIRE(called);
    }
}
