#include <catch2/catch_test_macros.hpp>
#include "test/stubs/WifiManagerStub.h"
#include "test/stubs/GpioStub.h"
#include "test/stubs/TimeStub.h"
#include "GpioPinRegister.h"

TEST_CASE("WifiManagerStub", "[WifiManagerStub]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    WifiManagerStub stub(GPIO_NUM_NC, gpioStub, pr, timeStub);

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

TEST_CASE("WifiManagerStub: status indicator LED", "[WifiManagerStub]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};

    SECTION("a configured pin is bound and defaults to LOW") {
        WifiManagerStub stub(GPIO_NUM_2, gpioStub, pr, timeStub);
        REQUIRE(pr.isPinBound(GPIO_NUM_2));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("GPIO_NUM_NC leaves no pin bound") {
        WifiManagerStub stub(GPIO_NUM_NC, gpioStub, pr, timeStub);
        REQUIRE_FALSE(pr.isPinBound(GPIO_NUM_2));
    }

    SECTION("goes HIGH on simulateConnected") {
        WifiManagerStub stub(GPIO_NUM_2, gpioStub, pr, timeStub);
        stub.simulateConnected();
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("goes LOW on simulateDisconnected after having been connected") {
        WifiManagerStub stub(GPIO_NUM_2, gpioStub, pr, timeStub);
        stub.simulateConnected();
        stub.simulateDisconnected();
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("goes LOW on simulateFailed after having been connected") {
        WifiManagerStub stub(GPIO_NUM_2, gpioStub, pr, timeStub);
        stub.simulateConnected();
        stub.simulateFailed();
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("goes LOW on free after having been connected") {
        WifiManagerStub stub(GPIO_NUM_2, gpioStub, pr, timeStub);
        stub.simulateConnected();
        stub.free();
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("updateOnboardingModeLedBlink does nothing without a configured pin") {
        WifiManagerStub stub(GPIO_NUM_NC, gpioStub, pr, timeStub);
        timeStub.setStubbedMillis(10000);
        stub.updateOnboardingModeLedBlink();
        SUCCEED("no crash, nothing to assert without a pin");
    }

    SECTION("updateOnboardingModeLedBlink does not toggle before 500ms elapse") {
        timeStub.setStubbedMillis(0);
        WifiManagerStub stub(GPIO_NUM_2, gpioStub, pr, timeStub);

        timeStub.setStubbedMillis(400);
        stub.updateOnboardingModeLedBlink();

        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("updateOnboardingModeLedBlink toggles once 500ms have elapsed") {
        timeStub.setStubbedMillis(0);
        WifiManagerStub stub(GPIO_NUM_2, gpioStub, pr, timeStub);

        timeStub.setStubbedMillis(500);
        stub.updateOnboardingModeLedBlink();
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));

        timeStub.setStubbedMillis(1000);
        stub.updateOnboardingModeLedBlink();
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));

        timeStub.setStubbedMillis(1500);
        stub.updateOnboardingModeLedBlink();
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("updateOnboardingModeLedBlink does not toggle twice within the same 500ms window") {
        timeStub.setStubbedMillis(0);
        WifiManagerStub stub(GPIO_NUM_2, gpioStub, pr, timeStub);

        timeStub.setStubbedMillis(500);
        stub.updateOnboardingModeLedBlink();
        stub.updateOnboardingModeLedBlink();
        stub.updateOnboardingModeLedBlink();

        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }
}
