#include <catch2/catch_test_macros.hpp>
#include "../main/include/GpioPinRegister.h"
#include "test/stubs/GpioStub.h"
#include "test/stubs/TimeStub.h"
#include "test/stubs/NVSStub.h"
#include "../main/include/StateMachine.h"
#include "../main/include/SettingsManager.h"
#include "../main/include/Valve.h"
#include "../main/include/ValveGroup.h"
#include "../main/include/ApiController.h"

TEST_CASE("ApiController: executeValveOperation", "[ApiController]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};

    std::array<Valve, 8> valves = {
        Valve(GPIO_NUM_0, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_1, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_2, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_3, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_4, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_5, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_6, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_7, gpioStub, timeStub, pr),
    };

    ValveGroup group(timeStub);
    REQUIRE(group.initialize(std::move(valves)));

    SECTION("Open opens the target valve") {
        REQUIRE(ApiController::executeValveOperation(group, ValveCommand{3, ValveAction::Open}));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("Close closes the target valve") {
        REQUIRE(ApiController::executeValveOperation(group, ValveCommand{3, ValveAction::Open}));
        REQUIRE(ApiController::executeValveOperation(group, ValveCommand{3, ValveAction::Close}));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("Open only affects the target valve") {
        REQUIRE(ApiController::executeValveOperation(group, ValveCommand{2, ValveAction::Open}));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_1) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("fails for an out-of-bounds index") {
        REQUIRE_FALSE(ApiController::executeValveOperation(group, ValveCommand{8, ValveAction::Open}));
    }
}

TEST_CASE("ApiController: executeValveOperation before group init", "[ApiController]") {
    TimeStub timeStub{};
    ValveGroup group(timeStub);

    SECTION("Open fails") {
        REQUIRE_FALSE(ApiController::executeValveOperation(group, ValveCommand{0, ValveAction::Open}));
    }

    SECTION("Close fails") {
        REQUIRE_FALSE(ApiController::executeValveOperation(group, ValveCommand{0, ValveAction::Close}));
    }
}

TEST_CASE("ApiController: saveWifiCredentials", "[ApiController]") {
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);
    StateMachine stateMachine;

    SECTION("stores ssid and password") {
        REQUIRE(ApiController::saveWifiCredentials(settings, stateMachine, WifiCredentials{"MyHomeWifi", "hunter2"}));

        const auto result = settings.retrieveWifiCredentials();
        REQUIRE(result.has_value());
        REQUIRE(result->ssid == "MyHomeWifi");
        REQUIRE(result->password == "hunter2");
    }

    SECTION("requests a shutdown on success") {
        REQUIRE(ApiController::saveWifiCredentials(settings, stateMachine, WifiCredentials{"MyHomeWifi", "hunter2"}));
        REQUIRE(stateMachine.getState() == STATE::SHUTTING_DOWN);
    }

    SECTION("fails before nvs is initialized") {
        NVSStub uninitializedNvs{};
        SettingsManager uninitializedSettings(uninitializedNvs);
        REQUIRE_FALSE(ApiController::saveWifiCredentials(uninitializedSettings, stateMachine, WifiCredentials{"MyHomeWifi", "hunter2"}));
    }

    SECTION("does not request a shutdown when saving fails") {
        NVSStub uninitializedNvs{};
        SettingsManager uninitializedSettings(uninitializedNvs);
        ApiController::saveWifiCredentials(uninitializedSettings, stateMachine, WifiCredentials{"MyHomeWifi", "hunter2"});
        REQUIRE(stateMachine.getState() == STATE::INITIALIZATION);
    }

    SECTION("a blank password keeps the previously stored one") {
        REQUIRE(ApiController::saveWifiCredentials(settings, stateMachine, WifiCredentials{"MyHomeWifi", "hunter2"}));
        REQUIRE(ApiController::saveWifiCredentials(settings, stateMachine, WifiCredentials{"MyOtherWifi", ""}));

        const auto result = settings.retrieveWifiCredentials();
        REQUIRE(result.has_value());
        REQUIRE(result->ssid == "MyOtherWifi");
        REQUIRE(result->password == "hunter2");
    }

    SECTION("a blank password stays blank when nothing was stored before") {
        REQUIRE(ApiController::saveWifiCredentials(settings, stateMachine, WifiCredentials{"OpenWifi", ""}));

        const auto result = settings.retrieveWifiCredentials();
        REQUIRE(result.has_value());
        REQUIRE(result->password.empty());
    }
}

TEST_CASE("ApiController: buildValveReport", "[ApiController]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};

    std::array<Valve, 8> valves = {
        Valve(GPIO_NUM_0, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_1, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
    };

    ValveGroup group(timeStub);
    REQUIRE(group.initialize(std::move(valves)));

    SECTION("lists all 8 valves, closed by default") {
        const std::string report = ApiController::buildValveReport(group);
        REQUIRE(report == "0:0\n1:0\n2:0\n3:0\n4:0\n5:0\n6:0\n7:0\n");
    }

    SECTION("reflects an open valve") {
        REQUIRE(ApiController::executeValveOperation(group, ValveCommand{1, ValveAction::Open}));
        const std::string report = ApiController::buildValveReport(group);
        REQUIRE(report == "0:0\n1:1\n2:0\n3:0\n4:0\n5:0\n6:0\n7:0\n");
    }
}

TEST_CASE("ApiController: settings report/form", "[ApiController]") {
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);
    StateMachine stateMachine;

    SECTION("buildSettingsReport reports blank fields when nothing was stored") {
        REQUIRE(ApiController::buildSettingsReport(settings) ==
            "device_name=\nmax_valve_runtime_min=\nwifi_ssid=\nwifi_password_set=0\n"
            "runtime_safety_enabled=\ncut_on_wifi_loss_enabled=\n");
    }

    SECTION("buildSettingsReport reports stored values") {
        REQUIRE(settings.storeTitle("Garden"));
        REQUIRE(settings.storeMaxValveRuntime(45));
        REQUIRE(settings.storeWifiCredentials(WifiCredentials{"MyHomeWifi", "hunter2"}));
        REQUIRE(settings.storeRuntimeSafetyEnabled(false));
        REQUIRE(settings.storeCutOnWifiLossEnabled(true));
        REQUIRE(ApiController::buildSettingsReport(settings) ==
            "device_name=Garden\nmax_valve_runtime_min=45\nwifi_ssid=MyHomeWifi\nwifi_password_set=1\n"
            "runtime_safety_enabled=0\ncut_on_wifi_loss_enabled=1\n");
    }

    SECTION("buildSettingsReport reports wifi_password_set=0 for an open network") {
        REQUIRE(settings.storeWifiCredentials(WifiCredentials{"OpenWifi", ""}));
        REQUIRE(ApiController::buildSettingsReport(settings) ==
            "device_name=\nmax_valve_runtime_min=\nwifi_ssid=OpenWifi\nwifi_password_set=0\n"
            "runtime_safety_enabled=\ncut_on_wifi_loss_enabled=\n");
    }

    SECTION("applySettingsForm stores the submitted values") {
        const std::unordered_map<std::string, std::string> form = {
            {"device_name", "Garden"},
            {"max_valve_runtime_min", "45"},
        };
        REQUIRE(ApiController::applySettingsForm(settings, stateMachine, form));
        REQUIRE(*settings.retrieveTitle() == "Garden");
        REQUIRE(*settings.retrieveMaxValveRuntime() == 45);
    }

    SECTION("applySettingsForm enables safety flags when their checkboxes are absent") {
        const std::unordered_map<std::string, std::string> form = {
            {"device_name", "Garden"},
            {"max_valve_runtime_min", "45"},
        };
        REQUIRE(ApiController::applySettingsForm(settings, stateMachine, form));
        REQUIRE(*settings.retrieveRuntimeSafetyEnabled());
        REQUIRE(*settings.retrieveCutOnWifiLossEnabled());
    }

    SECTION("applySettingsForm disables safety flags when their checkboxes are present") {
        const std::unordered_map<std::string, std::string> form = {
            {"device_name", "Garden"},
            {"max_valve_runtime_min", "45"},
            {"disable_runtime_safety", "1"},
            {"disable_cut_on_wifi_loss", "1"},
        };
        REQUIRE(ApiController::applySettingsForm(settings, stateMachine, form));
        REQUIRE_FALSE(*settings.retrieveRuntimeSafetyEnabled());
        REQUIRE_FALSE(*settings.retrieveCutOnWifiLossEnabled());
    }

    SECTION("applySettingsForm requests a shutdown on success") {
        const std::unordered_map<std::string, std::string> form = {
            {"device_name", "Garden"},
            {"max_valve_runtime_min", "45"},
        };
        REQUIRE(ApiController::applySettingsForm(settings, stateMachine, form));
        REQUIRE(stateMachine.getState() == STATE::SHUTTING_DOWN);
    }

    SECTION("applySettingsForm fails when device_name is missing") {
        const std::unordered_map<std::string, std::string> form = {
            {"max_valve_runtime_min", "45"},
        };
        REQUIRE_FALSE(ApiController::applySettingsForm(settings, stateMachine, form));
    }

    SECTION("applySettingsForm fails when max_valve_runtime_min is not a number") {
        const std::unordered_map<std::string, std::string> form = {
            {"device_name", "Garden"},
            {"max_valve_runtime_min", "abc"},
        };
        REQUIRE_FALSE(ApiController::applySettingsForm(settings, stateMachine, form));
    }

    SECTION("applySettingsForm stores nothing when a later field fails validation") {
        const std::unordered_map<std::string, std::string> form = {
            {"device_name", "Garden"},
            {"max_valve_runtime_min", "abc"},
        };
        REQUIRE_FALSE(ApiController::applySettingsForm(settings, stateMachine, form));
        REQUIRE_FALSE(settings.retrieveTitle().has_value());
    }
}

TEST_CASE("ApiController: advanced settings report/form", "[ApiController]") {
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);
    StateMachine stateMachine;

    SECTION("buildAdvancedSettingsReport reports blank fields when nothing was stored") {
        const std::string report = ApiController::buildAdvancedSettingsReport(settings);
        REQUIRE(report == "num_valves=\ngpio0=\ngpio1=\ngpio2=\ngpio3=\ngpio4=\ngpio5=\ngpio6=\ngpio7=\n");
    }

    SECTION("buildAdvancedSettingsReport reports stored values") {
        REQUIRE(settings.storeNumValves(4));
        REQUIRE(settings.storeValveActuatorGpioPins({
            GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_15, GPIO_NUM_16,
            GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
        }));
        const std::string report = ApiController::buildAdvancedSettingsReport(settings);
        REQUIRE(report == "num_valves=4\ngpio0=13\ngpio1=14\ngpio2=15\ngpio3=16\ngpio4=\ngpio5=\ngpio6=\ngpio7=\n");
    }

    SECTION("applyAdvancedSettingsForm stores the submitted values") {
        const std::unordered_map<std::string, std::string> form = {
            {"num_valves", "4"},
            {"gpio0", "13"},
            {"gpio1", "14"},
            {"gpio2", "15"},
            {"gpio3", "16"},
        };
        REQUIRE(ApiController::applyAdvancedSettingsForm(settings, stateMachine, form));
        REQUIRE(*settings.retrieveNumValves() == 4);

        const auto pins = settings.retrieveValveActuatorGpioPins();
        REQUIRE(pins.has_value());
        REQUIRE((*pins)[0] == GPIO_NUM_13);
        REQUIRE((*pins)[3] == GPIO_NUM_16);
        REQUIRE((*pins)[4] == GPIO_NUM_NC);
        REQUIRE((*pins)[7] == GPIO_NUM_NC);
    }

    SECTION("applyAdvancedSettingsForm unsets pins beyond the new valve count") {
        REQUIRE(settings.storeNumValves(4));
        REQUIRE(settings.storeValveActuatorGpioPins({
            GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_15,
            GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
        }));

        const std::unordered_map<std::string, std::string> form = {
            {"num_valves", "2"},
            {"gpio0", "12"},
            {"gpio1", "13"},
            {"gpio2", "14"},
            {"gpio3", "15"},
        };
        REQUIRE(ApiController::applyAdvancedSettingsForm(settings, stateMachine, form));

        const auto pins = settings.retrieveValveActuatorGpioPins();
        REQUIRE(pins.has_value());
        REQUIRE((*pins)[0] == GPIO_NUM_12);
        REQUIRE((*pins)[1] == GPIO_NUM_13);
        REQUIRE((*pins)[2] == GPIO_NUM_NC);
        REQUIRE((*pins)[3] == GPIO_NUM_NC);
    }

    SECTION("applyAdvancedSettingsForm requests a shutdown on success") {
        const std::unordered_map<std::string, std::string> form = {{"num_valves", "4"}};
        REQUIRE(ApiController::applyAdvancedSettingsForm(settings, stateMachine, form));
        REQUIRE(stateMachine.getState() == STATE::SHUTTING_DOWN);
    }

    SECTION("applyAdvancedSettingsForm fails when num_valves is missing") {
        const std::unordered_map<std::string, std::string> form = {};
        REQUIRE_FALSE(ApiController::applyAdvancedSettingsForm(settings, stateMachine, form));
    }

    SECTION("applyAdvancedSettingsForm fails when num_valves is not a number") {
        const std::unordered_map<std::string, std::string> form = {{"num_valves", "abc"}};
        REQUIRE_FALSE(ApiController::applyAdvancedSettingsForm(settings, stateMachine, form));
    }
}
