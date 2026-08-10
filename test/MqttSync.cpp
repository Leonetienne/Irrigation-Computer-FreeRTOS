#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <array>
#include "GpioPinRegister.h"
#include "test/stubs/GpioStub.h"
#include "test/stubs/TimeStub.h"
#include "test/stubs/NVSStub.h"
#include "test/stubs/MqttStub.h"
#include "Valve.h"
#include "ValveGroup.h"
#include "SettingsManager.h"
#include "MqttSync.h"
#include "MqttTopics.h"

namespace {

std::array<Valve, 8> makeValves(GpioStub& gpioStub, TimeStub& timeStub, GpioPinRegister& pr) {
    return {
        Valve(GPIO_NUM_0, GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_1, GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, GPIO_NUM_NC, gpioStub, timeStub, pr),
    };
}

}

TEST_CASE("MqttSync: connect", "[MqttSync]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    REQUIRE(valveGroup.initialize(makeValves(gpioStub, timeStub, pr)));
    MqttStub mqtt{};
    MqttSync sync(mqtt, valveGroup, settings);
    sync.begin();

    SECTION("fails when no broker uri is configured") {
        REQUIRE_FALSE(sync.connect());
        REQUIRE(mqtt.getBeginCallCount() == 0);
    }

    SECTION("connects using the configured broker and device name as lwt topic") {
        REQUIRE(settings.storeTitle("my_device"));
        REQUIRE(settings.storeMqttBrokerConfig({"mqtt://broker:1883", "user", "pass"}));

        REQUIRE(sync.connect());
        REQUIRE(mqtt.getBeginCallCount() == 1);
        REQUIRE(mqtt.getLastConnectOptions().brokerUri == "mqtt://broker:1883");
        REQUIRE(mqtt.getLastConnectOptions().username == "user");
        REQUIRE(mqtt.getLastConnectOptions().password == "pass");
        REQUIRE(mqtt.getLastConnectOptions().lwtTopic == "tele/irrigation/my_device/LWT");
        REQUIRE(mqtt.getLastConnectOptions().lwtMessage == "Offline");
    }

    SECTION("fails when called twice") {
        REQUIRE(settings.storeMqttBrokerConfig({"mqtt://broker:1883", "", ""}));
        REQUIRE(sync.connect());
        REQUIRE_FALSE(sync.connect());
        REQUIRE(mqtt.getBeginCallCount() == 1);
    }
}

TEST_CASE("MqttSync: onMqttConnected", "[MqttSync]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    REQUIRE(valveGroup.initialize(makeValves(gpioStub, timeStub, pr)));
    REQUIRE(settings.storeTitle("my_device"));
    REQUIRE(settings.storeMqttBrokerConfig({"mqtt://broker:1883", "", ""}));

    MqttStub mqtt{};
    MqttSync sync(mqtt, valveGroup, settings);
    sync.begin();
    REQUIRE(sync.connect());

    mqtt.simulateConnected();

    SECTION("publishes availability retained") {
        const auto& messages = mqtt.getPublishedMessages();
        const auto it = std::find_if(messages.begin(), messages.end(), [](const auto& m) {
            return m.topic == "tele/irrigation/my_device/LWT";
        });
        REQUIRE(it != messages.end());
        REQUIRE(it->payload == "Online");
        REQUIRE(it->retain);
    }

    SECTION("subscribes command topics only for operable valves") {
        const auto& subs = mqtt.getSubscriptions();
        REQUIRE(subs.size() == 2);
        REQUIRE(subs[0].topic == "cmnd/irrigation/my_device/0/POWER");
        REQUIRE(subs[1].topic == "cmnd/irrigation/my_device/1/POWER");
    }

    SECTION("publishes discovery and current (closed) state for operable valves") {
        const auto& messages = mqtt.getPublishedMessages();

        const auto discovery = std::find_if(messages.begin(), messages.end(), [](const auto& m) {
            return m.topic == "homeassistant/switch/my_device/my_device_0/config";
        });
        REQUIRE(discovery != messages.end());

        const auto state = std::find_if(messages.begin(), messages.end(), [](const auto& m) {
            return m.topic == "stat/irrigation/my_device/0/POWER";
        });
        REQUIRE(state != messages.end());
        REQUIRE(state->payload == "OFF");
    }
}

TEST_CASE("MqttSync: onMqttMessage applies valve commands", "[MqttSync]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    REQUIRE(valveGroup.initialize(makeValves(gpioStub, timeStub, pr)));
    REQUIRE(settings.storeTitle("my_device"));
    REQUIRE(settings.storeMqttBrokerConfig({"mqtt://broker:1883", "", ""}));

    MqttStub mqtt{};
    MqttSync sync(mqtt, valveGroup, settings);
    sync.begin();
    REQUIRE(sync.connect());
    mqtt.simulateConnected();

    SECTION("ON opens the addressed valve") {
        mqtt.simulateMessage("cmnd/irrigation/my_device/0/POWER", "ON");
        REQUIRE(valveGroup.getValveOpenState(0) == true);
    }

    SECTION("OFF closes the addressed valve") {
        mqtt.simulateMessage("cmnd/irrigation/my_device/0/POWER", "ON");
        mqtt.simulateMessage("cmnd/irrigation/my_device/0/POWER", "OFF");
        REQUIRE(valveGroup.getValveOpenState(0) == false);
    }

    SECTION("ignores commands for a non-operable valve index") {
        mqtt.simulateMessage("cmnd/irrigation/my_device/2/POWER", "ON");
        REQUIRE(valveGroup.getValveOpenState(2).has_value() == false);
    }

    SECTION("ignores an unrecognized payload") {
        mqtt.simulateMessage("cmnd/irrigation/my_device/0/POWER", "TOGGLE");
        REQUIRE(valveGroup.getValveOpenState(0) == false);
    }

    SECTION("ignores a topic for a different device") {
        mqtt.simulateMessage("cmnd/irrigation/other_device/0/POWER", "ON");
        REQUIRE(valveGroup.getValveOpenState(0) == false);
    }
}

TEST_CASE("MqttSync: onMqttDisconnected", "[MqttSync]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    REQUIRE(valveGroup.initialize(makeValves(gpioStub, timeStub, pr)));
    REQUIRE(settings.storeTitle("my_device"));
    REQUIRE(settings.storeMqttBrokerConfig({"mqtt://broker:1883", "", ""}));

    MqttStub mqtt{};
    MqttSync sync(mqtt, valveGroup, settings);
    sync.begin();
    REQUIRE(sync.connect());
    mqtt.simulateConnected();
    REQUIRE(valveGroup.open(0));

    SECTION("closes open valves by default") {
        mqtt.simulateDisconnected();
        REQUIRE(valveGroup.getValveOpenState(0) == false);
    }

    SECTION("leaves valves open when cut-on-mqtt-loss is disabled") {
        REQUIRE(settings.storeCutOnMqttLossEnabled(false));
        mqtt.simulateDisconnected();
        REQUIRE(valveGroup.getValveOpenState(0) == true);
    }
}

TEST_CASE("MqttSync: pollPublishStateChanges", "[MqttSync]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);
    ValveGroup valveGroup(timeStub, settings);
    REQUIRE(valveGroup.initialize(makeValves(gpioStub, timeStub, pr)));
    REQUIRE(settings.storeTitle("my_device"));
    REQUIRE(settings.storeMqttBrokerConfig({"mqtt://broker:1883", "", ""}));

    MqttStub mqtt{};
    MqttSync sync(mqtt, valveGroup, settings);
    sync.begin();

    SECTION("does nothing before connect") {
        sync.pollPublishStateChanges();
        REQUIRE(mqtt.getPublishedMessages().empty());
    }

    SECTION("republishes state only when it changes") {
        REQUIRE(sync.connect());
        mqtt.simulateConnected();
        const std::size_t afterConnectCount = mqtt.getPublishedMessages().size();

        sync.pollPublishStateChanges();
        REQUIRE(mqtt.getPublishedMessages().size() == afterConnectCount);

        REQUIRE(valveGroup.open(0));
        sync.pollPublishStateChanges();

        const auto& messages = mqtt.getPublishedMessages();
        REQUIRE(messages.size() == afterConnectCount + 1);
        REQUIRE(messages.back().topic == "stat/irrigation/my_device/0/POWER");
        REQUIRE(messages.back().payload == "ON");
    }
}
