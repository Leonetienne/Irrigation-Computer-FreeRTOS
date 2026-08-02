#include <catch2/catch_test_macros.hpp>
#include "test/stubs/NVSStub.h"
#include "../main/include/SettingsManager.h"

TEST_CASE("SettingsManager: wifi credentials", "[SettingsManager]") {
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

    SECTION("round trip stores and retrieves credentials") {
        REQUIRE(settings.storeWifiCredentials(WifiCredentials{"MyHomeWifi", "hunter2"}));

        const auto result = settings.retrieveWifiCredentials();
        REQUIRE(result.has_value());
        REQUIRE(result->ssid == "MyHomeWifi");
        REQUIRE(result->password == "hunter2");
    }

    SECTION("retrieve fails when nothing was stored") {
        REQUIRE_FALSE(settings.retrieveWifiCredentials().has_value());
    }

    SECTION("store fails when nvs is not ready") {
        NVSStub uninitializedNvs{};
        SettingsManager uninitializedSettings(uninitializedNvs);
        REQUIRE_FALSE(uninitializedSettings.storeWifiCredentials(WifiCredentials{"MyHomeWifi", "hunter2"}));
    }
}

TEST_CASE("SettingsManager: title", "[SettingsManager]") {
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

    SECTION("round trip stores and retrieves the title") {
        REQUIRE(settings.storeTitle("Irrigation Computer"));

        const auto result = settings.retrieveTitle();
        REQUIRE(result.has_value());
        REQUIRE(*result == "Irrigation Computer");
    }

    SECTION("retrieve fails when nothing was stored") {
        REQUIRE_FALSE(settings.retrieveTitle().has_value());
    }
}

TEST_CASE("SettingsManager: num valves", "[SettingsManager]") {
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

    SECTION("round trip stores and retrieves") {
        REQUIRE(settings.storeNumValves(4));

        const auto result = settings.retrieveNumValves();
        REQUIRE(result.has_value());
        REQUIRE(*result == 4);
    }

    SECTION("retrieve fails when nothing was stored") {
        REQUIRE_FALSE(settings.retrieveNumValves().has_value());
    }
}

TEST_CASE("SettingsManager: max valve runtime", "[SettingsManager]") {
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

    SECTION("round trip stores and retrieves") {
        REQUIRE(settings.storeMaxValveRuntime(30));

        const auto result = settings.retrieveMaxValveRuntime();
        REQUIRE(result.has_value());
        REQUIRE(*result == 30);
    }

    SECTION("retrieve fails when nothing was stored") {
        REQUIRE_FALSE(settings.retrieveMaxValveRuntime().has_value());
    }
}

TEST_CASE("SettingsManager: valve actuator gpio pins", "[SettingsManager]") {
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

    SECTION("round trip stores and retrieves all 8 pins") {
        constexpr std::array<gpio_num_t, 8> pins = {
            GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_15, GPIO_NUM_16,
            GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
        };
        REQUIRE(settings.storeValveActuatorGpioPins(pins));

        const auto result = settings.retrieveValveActuatorGpioPins();
        REQUIRE(result.has_value());
        REQUIRE(*result == pins);
    }

    SECTION("round trip preserves the negative NC sentinel for every valve") {
        constexpr std::array<gpio_num_t, 8> pins = {
            GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
            GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
        };
        REQUIRE(settings.storeValveActuatorGpioPins(pins));

        const auto result = settings.retrieveValveActuatorGpioPins();
        REQUIRE(result.has_value());
        REQUIRE(*result == pins);
    }

    SECTION("round trip with the highest supported pin numbers") {
        constexpr std::array<gpio_num_t, 8> pins = {
            GPIO_NUM_39, GPIO_NUM_38, GPIO_NUM_37, GPIO_NUM_36,
            GPIO_NUM_35, GPIO_NUM_34, GPIO_NUM_33, GPIO_NUM_32,
        };
        REQUIRE(settings.storeValveActuatorGpioPins(pins));

        const auto result = settings.retrieveValveActuatorGpioPins();
        REQUIRE(result.has_value());
        REQUIRE(*result == pins);
    }

    SECTION("retrieve fails when nothing was stored") {
        REQUIRE_FALSE(settings.retrieveValveActuatorGpioPins().has_value());
    }
}

TEST_CASE("SettingsManager: safety flags", "[SettingsManager]") {
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

    SECTION("round trip stores and retrieves runtime safety enabled") {
        REQUIRE(settings.storeRuntimeSafetyEnabled(false));

        const auto result = settings.retrieveRuntimeSafetyEnabled();
        REQUIRE(result.has_value());
        REQUIRE_FALSE(*result);
    }

    SECTION("retrieve fails when runtime safety was never stored") {
        REQUIRE_FALSE(settings.retrieveRuntimeSafetyEnabled().has_value());
    }

    SECTION("round trip stores and retrieves cut on wifi loss enabled") {
        REQUIRE(settings.storeCutOnWifiLossEnabled(true));

        const auto result = settings.retrieveCutOnWifiLossEnabled();
        REQUIRE(result.has_value());
        REQUIRE(*result);
    }

    SECTION("retrieve fails when cut on wifi loss was never stored") {
        REQUIRE_FALSE(settings.retrieveCutOnWifiLossEnabled().has_value());
    }
}

TEST_CASE("SettingsManager: move", "[SettingsManager]") {
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

    SettingsManager moved(std::move(settings));

    SECTION("moved-to instance operates on the same nvs") {
        REQUIRE(moved.storeNumValves(7));

        const auto result = moved.retrieveNumValves();
        REQUIRE(result.has_value());
        REQUIRE(*result == 7);
    }
}
