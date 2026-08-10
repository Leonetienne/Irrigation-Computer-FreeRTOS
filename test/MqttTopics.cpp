#include <catch2/catch_test_macros.hpp>
#include "MqttTopics.h"

TEST_CASE("MqttTopics: topic builders", "[MqttTopics]") {
    REQUIRE(MqttTopics::commandTopic("my_device", 3) == "cmnd/irrigation/my_device/3/POWER");
    REQUIRE(MqttTopics::stateTopic("my_device", 3) == "stat/irrigation/my_device/3/POWER");
    REQUIRE(MqttTopics::availabilityTopic("my_device") == "tele/irrigation/my_device/LWT");
    REQUIRE(
        MqttTopics::discoveryTopic("my_node", "my_device", 3) ==
        "homeassistant/switch/my_node/my_device_3/config"
    );
}

TEST_CASE("MqttTopics: buildDiscoveryPayload", "[MqttTopics]") {
    const std::string payload = MqttTopics::buildDiscoveryPayload("my_node", "my_device", 2);

    REQUIRE(payload.find(R"("name":"my_device Valve 2")") != std::string::npos);
    REQUIRE(payload.find(R"("unique_id":"my_node_my_device_2")") != std::string::npos);
    REQUIRE(payload.find(R"("state_topic":"stat/irrigation/my_device/2/POWER")") != std::string::npos);
    REQUIRE(payload.find(R"("command_topic":"cmnd/irrigation/my_device/2/POWER")") != std::string::npos);
    REQUIRE(payload.find(R"("availability_topic":"tele/irrigation/my_device/LWT")") != std::string::npos);
    REQUIRE(payload.find(R"("identifiers":["my_node_my_device"])") != std::string::npos);
}

TEST_CASE("MqttTopics: parseCommandTopic", "[MqttTopics]") {
    SECTION("matches a valid command topic") {
        const auto index = MqttTopics::parseCommandTopic("my_device", "cmnd/irrigation/my_device/5/POWER");
        REQUIRE(index.has_value());
        REQUIRE(*index == 5);
    }

    SECTION("matches index 0") {
        const auto index = MqttTopics::parseCommandTopic("my_device", "cmnd/irrigation/my_device/0/POWER");
        REQUIRE(index.has_value());
        REQUIRE(*index == 0);
    }

    SECTION("rejects a different device's topic") {
        REQUIRE_FALSE(MqttTopics::parseCommandTopic("my_device", "cmnd/irrigation/other_device/5/POWER").has_value());
    }

    SECTION("rejects a state topic") {
        REQUIRE_FALSE(MqttTopics::parseCommandTopic("my_device", "stat/irrigation/my_device/5/POWER").has_value());
    }

    SECTION("rejects a non-numeric index") {
        REQUIRE_FALSE(MqttTopics::parseCommandTopic("my_device", "cmnd/irrigation/my_device/foo/POWER").has_value());
    }

    SECTION("rejects a missing index") {
        REQUIRE_FALSE(MqttTopics::parseCommandTopic("my_device", "cmnd/irrigation/my_device//POWER").has_value());
    }

    SECTION("rejects an unrelated topic") {
        REQUIRE_FALSE(MqttTopics::parseCommandTopic("my_device", "homeassistant/status").has_value());
    }
}

TEST_CASE("MqttTopics: parsePayload", "[MqttTopics]") {
    REQUIRE(MqttTopics::parsePayload("ON") == std::optional<bool>(true));
    REQUIRE(MqttTopics::parsePayload("OFF") == std::optional<bool>(false));
    REQUIRE_FALSE(MqttTopics::parsePayload("on").has_value());
    REQUIRE_FALSE(MqttTopics::parsePayload("").has_value());
    REQUIRE_FALSE(MqttTopics::parsePayload("TOGGLE").has_value());
}
