#include <catch2/catch_test_macros.hpp>
#include "System.h"
#include "GpioPinRegister.h"
#include "StateMachine.h"
#include "SettingsManager.h"
#include "ValveGroup.h"
#include "test/stubs/GpioStub.h"
#include "test/stubs/TimeStub.h"
#include "test/stubs/NVSStub.h"
#include "test/stubs/WifiManagerStub.h"
#include "test/stubs/HttpServerStub.h"
#include "test/stubs/MqttStub.h"
#include "test/stubs/SystemStub.h"
#include "MqttSync.h"

TEST_CASE("System: init", "[System]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    WifiManagerStub wifiMan(GPIO_NUM_NC, gpioStub, pr, timeStub);
    HttpServerStub httpServer{};
    StateMachine stateMachine{};
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    MqttStub mqttStub(GPIO_NUM_NC, gpioStub, pr, timeStub);
    MqttSync mqttSync(mqttStub, valveGroup, settings);

    System system(stateMachine, pr, gpioStub, timeStub, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync);

    SECTION("without stored wifi credentials enters onboarding and starts the http server") {
        system.init();

        REQUIRE(stateMachine.getState() == STATE::WIFI_ONBOARDING);
        REQUIRE(wifiMan.getBeginOnboardingWifiCallCount() == 1);
        REQUIRE(wifiMan.getBeginUserWifiCallCount() == 0);
        REQUIRE(httpServer.test_getBeginCallCount() == 1);
        REQUIRE(httpServer.test_isRunning());
        REQUIRE(valveGroup.isReady());
    }

    SECTION("with stored wifi credentials waits for connection without starting the http server yet") {
        REQUIRE(settings.storeWifiCredentials({"my-ssid", "my-password"}));

        system.init();

        REQUIRE(stateMachine.getState() == STATE::WAIT_WIFI_CONNECTION);
        REQUIRE(wifiMan.getBeginUserWifiCallCount() == 1);
        REQUIRE(wifiMan.getLastSsid() == "my-ssid");
        REQUIRE(wifiMan.getLastPassword() == "my-password");
        REQUIRE(httpServer.test_getBeginCallCount() == 0);
        REQUIRE_FALSE(httpServer.test_isRunning());
    }

    SECTION("wires configured valve pins into the valve group") {
        REQUIRE(settings.storeNumValves(2));
        REQUIRE(settings.storeValveActuatorGpioPins({
            GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_NC, GPIO_NUM_NC,
            GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC
        }));

        system.init();

        REQUIRE(valveGroup.isValveOperable(0));
        REQUIRE(valveGroup.isValveOperable(1));
        REQUIRE_FALSE(valveGroup.isValveOperable(2));
    }

    SECTION("wires configured valve status indicator pins into the valve group") {
        REQUIRE(settings.storeNumValves(2));
        REQUIRE(settings.storeValveActuatorGpioPins({
            GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_NC, GPIO_NUM_NC,
            GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC
        }));
        REQUIRE(settings.storeValveIndicatorGpioPins({
            GPIO_NUM_20, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
            GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC
        }));

        system.init();
        REQUIRE(valveGroup.open(0));

        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_20) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("leaves a leftover valve indicator pin unbound when valve leds are disabled") {
        REQUIRE(settings.storeNumValves(2));
        REQUIRE(settings.storeValveActuatorGpioPins({
            GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_NC, GPIO_NUM_NC,
            GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC
        }));
        REQUIRE(settings.storeValveIndicatorGpioPins({
            GPIO_NUM_20, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
            GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC
        }));
        REQUIRE(settings.storeValveLedsEnabled(false));

        system.init();
        REQUIRE(valveGroup.open(0));

        REQUIRE_FALSE(pr.isPinBound(GPIO_NUM_20));
    }
}

TEST_CASE("System: free", "[System]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    WifiManagerStub wifiMan(GPIO_NUM_NC, gpioStub, pr, timeStub);
    HttpServerStub httpServer{};
    StateMachine stateMachine{};
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    MqttStub mqttStub(GPIO_NUM_NC, gpioStub, pr, timeStub);
    MqttSync mqttSync(mqttStub, valveGroup, settings);

    System system(stateMachine, pr, gpioStub, timeStub, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync);

    SECTION("fails before init") {
        REQUIRE_FALSE(system.free());
    }

    SECTION("succeeds after init and releases dependencies") {
        system.init();
        REQUIRE(httpServer.test_isRunning());

        REQUIRE(system.free());

        REQUIRE_FALSE(httpServer.test_isRunning());
        REQUIRE_FALSE(valveGroup.isReady());
    }

    SECTION("fails when called twice") {
        system.init();
        REQUIRE(system.free());
        REQUIRE_FALSE(system.free());
    }
}

TEST_CASE("System: onWifiConnected", "[System]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    WifiManagerStub wifiMan(GPIO_NUM_NC, gpioStub, pr, timeStub);
    HttpServerStub httpServer{};
    StateMachine stateMachine{};
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    MqttStub mqttStub(GPIO_NUM_NC, gpioStub, pr, timeStub);
    MqttSync mqttSync(mqttStub, valveGroup, settings);

    System system(stateMachine, pr, gpioStub, timeStub, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync);

    // no stored credentials -> onboarding, http server already begun once by init()
    system.init();

    SECTION("wifi connect transitions to OPERATIONAL and (re-)starts the http server") {
        wifiMan.simulateConnected();

        REQUIRE(stateMachine.getState() == STATE::OPERATIONAL);
        REQUIRE(httpServer.test_getBeginCallCount() == 2);
    }
}

TEST_CASE("System: onWifiDisconnected", "[System]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    WifiManagerStub wifiMan(GPIO_NUM_NC, gpioStub, pr, timeStub);
    HttpServerStub httpServer{};
    StateMachine stateMachine{};
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    MqttStub mqttStub(GPIO_NUM_NC, gpioStub, pr, timeStub);
    MqttSync mqttSync(mqttStub, valveGroup, settings);

    System system(stateMachine, pr, gpioStub, timeStub, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync);

    REQUIRE(settings.storeNumValves(1));
    REQUIRE(settings.storeValveActuatorGpioPins({
        GPIO_NUM_0, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
        GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC
    }));

    system.init();
    REQUIRE(valveGroup.open(0));

    SECTION("closes open valves and stops the http server by default") {
        wifiMan.simulateDisconnected();

        REQUIRE(stateMachine.getState() == STATE::WAIT_WIFI_CONNECTION);
        REQUIRE(valveGroup.getValveOpenState(0) == false);
        REQUIRE_FALSE(httpServer.test_isRunning());
    }

    SECTION("leaves valves open when cut-on-wifi-loss is disabled") {
        REQUIRE(settings.storeCutOnWifiLossEnabled(false));

        wifiMan.simulateDisconnected();

        REQUIRE(valveGroup.getValveOpenState(0) == true);
    }
}

TEST_CASE("System: onWifiFailed defers the onboarding fallback to update()", "[System]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    WifiManagerStub wifiMan(GPIO_NUM_NC, gpioStub, pr, timeStub);
    HttpServerStub httpServer{};
    StateMachine stateMachine{};
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    MqttStub mqttStub(GPIO_NUM_NC, gpioStub, pr, timeStub);
    MqttSync mqttSync(mqttStub, valveGroup, settings);

    System system(stateMachine, pr, gpioStub, timeStub, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync);

    system.init();
    wifiMan.simulateConnected();
    wifiMan.simulateFailed();

    SECTION("state does not change until update() runs") {
        REQUIRE(stateMachine.getState() == STATE::OPERATIONAL);
    }

    SECTION("update() restarts onboarding wifi and the http server") {
        system.update();

        REQUIRE(stateMachine.getState() == STATE::WIFI_ONBOARDING);
        REQUIRE(wifiMan.getBeginOnboardingWifiCallCount() == 2); // once from init(), once from the fallback
        REQUIRE(httpServer.test_isRunning());
    }

    SECTION("the fallback is only applied once") {
        system.update();
        stateMachine.setState(STATE::OPERATIONAL); // simulate having reconnected afterwards
        system.update();

        REQUIRE(stateMachine.getState() == STATE::OPERATIONAL);
        REQUIRE(wifiMan.getBeginOnboardingWifiCallCount() == 2);
    }
}

TEST_CASE("System: update polls valve auto-close timeouts", "[System]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    WifiManagerStub wifiMan(GPIO_NUM_NC, gpioStub, pr, timeStub);
    HttpServerStub httpServer{};
    StateMachine stateMachine{};
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    MqttStub mqttStub(GPIO_NUM_NC, gpioStub, pr, timeStub);
    MqttSync mqttSync(mqttStub, valveGroup, settings);

    System system(stateMachine, pr, gpioStub, timeStub, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync);

    REQUIRE(settings.storeNumValves(1));
    REQUIRE(settings.storeValveActuatorGpioPins({
        GPIO_NUM_0, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
        GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC
    }));
    REQUIRE(settings.storeRuntimeSafetyEnabled(true));
    REQUIRE(settings.storeMaxValveRuntime(1)); // minutes -> 60s

    system.init();
    REQUIRE(valveGroup.open(0));

    timeStub.setStubbedTime(timeStub.getTime() + 61);

    SECTION("closes the valve once the throttled poll interval is reached") {
        // auto-close is only actually polled every 200 update() ticks (~2s at the ~10ms
        // production loop cadence), not on every tick - see System::VALVE_POLL_INTERVAL_TICKS
        for (int i = 0; i < 200; ++i) {
            system.update();
        }

        REQUIRE(valveGroup.getValveOpenState(0) == false);
    }

    SECTION("does not close the valve before the throttled poll interval is reached") {
        for (int i = 0; i < 199; ++i) {
            system.update();
        }

        REQUIRE(valveGroup.getValveOpenState(0) == true);
    }
}

TEST_CASE("System: update polls the onboarding-mode led blink only during onboarding", "[System]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    timeStub.setStubbedMillis(0);
    WifiManagerStub wifiMan(GPIO_NUM_2, gpioStub, pr, timeStub);
    HttpServerStub httpServer{};
    StateMachine stateMachine{};
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    MqttStub mqttStub(GPIO_NUM_NC, gpioStub, pr, timeStub);
    MqttSync mqttSync(mqttStub, valveGroup, settings);

    System system(stateMachine, pr, gpioStub, timeStub, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync);

    SECTION("blinks the led while in onboarding mode") {
        system.init(); // no stored credentials -> WIFI_ONBOARDING
        REQUIRE(stateMachine.getState() == STATE::WIFI_ONBOARDING);

        timeStub.setStubbedMillis(500);
        system.update();
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));

        timeStub.setStubbedMillis(1000);
        system.update();
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("does not blink the led outside onboarding mode") {
        REQUIRE(settings.storeWifiCredentials({"my-ssid", "my-password"}));

        system.init(); // stored credentials -> WAIT_WIFI_CONNECTION, not onboarding
        REQUIRE(stateMachine.getState() == STATE::WAIT_WIFI_CONNECTION);

        timeStub.setStubbedMillis(500);
        system.update();
        timeStub.setStubbedMillis(1000);
        system.update();

        // stays at its initial LOW - update() never polled the blink outside onboarding
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }
}

TEST_CASE("System: update polls the mqtt activity led pulse", "[System]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    timeStub.setStubbedMillis(0);
    WifiManagerStub wifiMan(GPIO_NUM_NC, gpioStub, pr, timeStub);
    HttpServerStub httpServer{};
    StateMachine stateMachine{};
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    MqttStub mqttStub(GPIO_NUM_3, gpioStub, pr, timeStub);
    MqttSync mqttSync(mqttStub, valveGroup, settings);

    System system(stateMachine, pr, gpioStub, timeStub, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync);

    system.init();

    SECTION("turns the led off 200ms after a publish, regardless of system state") {
        mqttStub.publish("stat/valve/0", "ON", 1, true);
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));

        timeStub.setStubbedMillis(199);
        system.update();
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));

        timeStub.setStubbedMillis(200);
        system.update();
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("turns the led off 200ms after a received message") {
        mqttStub.simulateMessage("cmnd/irrigation/x/0/POWER", "ON");
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));

        timeStub.setStubbedMillis(200);
        system.update();
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }
}

TEST_CASE("System: wifi reset button", "[System]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    timeStub.setStubbedMillis(0);
    WifiManagerStub wifiMan(GPIO_NUM_NC, gpioStub, pr, timeStub);
    HttpServerStub httpServer{};
    StateMachine stateMachine{};
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    MqttStub mqttStub(GPIO_NUM_NC, gpioStub, pr, timeStub);
    MqttSync mqttSync(mqttStub, valveGroup, settings);

    System system(stateMachine, pr, gpioStub, timeStub, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync);

    REQUIRE(settings.storeWifiCredentials({"my-ssid", "my-password"}));
    system.init(); // stored credentials -> WAIT_WIFI_CONNECTION, not onboarding

    SECTION("holding it for the full threshold wipes credentials and falls back to onboarding") {
        gpioStub.test_setInputLevel(GPIO_NUM_0, 0); // pressed (active low)
        system.update();

        timeStub.setStubbedMillis(5000);
        system.update();

        REQUIRE(stateMachine.getState() == STATE::WIFI_ONBOARDING);
        REQUIRE_FALSE(settings.retrieveWifiCredentials().has_value());
        REQUIRE(wifiMan.getBeginOnboardingWifiCallCount() == 1);
        REQUIRE(httpServer.test_isRunning());
    }

    SECTION("releasing it before the threshold does not trigger a reset") {
        gpioStub.test_setInputLevel(GPIO_NUM_0, 0); // pressed
        system.update();

        timeStub.setStubbedMillis(4999);
        system.update();

        gpioStub.test_setInputLevel(GPIO_NUM_0, 1); // released
        system.update();

        timeStub.setStubbedMillis(5000);
        system.update();

        REQUIRE(stateMachine.getState() == STATE::WAIT_WIFI_CONNECTION);
        REQUIRE(settings.retrieveWifiCredentials().has_value());
        REQUIRE(wifiMan.getBeginOnboardingWifiCallCount() == 0);
    }

    SECTION("a held button only triggers once, not on every subsequent tick") {
        gpioStub.test_setInputLevel(GPIO_NUM_0, 0); // pressed
        system.update();

        timeStub.setStubbedMillis(5000);
        system.update();
        system.update();
        system.update();

        REQUIRE(wifiMan.getBeginOnboardingWifiCallCount() == 1);
    }
}

TEST_CASE("SystemStub: getSystem wires a usable System", "[System][SystemStub]") {
    System& system = getSystem();

    system.init();
    REQUIRE(system.free());
}
