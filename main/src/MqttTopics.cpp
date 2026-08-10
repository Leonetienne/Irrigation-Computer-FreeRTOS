#include "MqttTopics.h"

std::string MqttTopics::commandTopic(const std::string& deviceName, std::size_t index) noexcept {
    return "cmnd/irrigation/" + deviceName + "/" + std::to_string(index) + "/POWER";
}

std::string MqttTopics::stateTopic(const std::string& deviceName, std::size_t index) noexcept {
    return "stat/irrigation/" + deviceName + "/" + std::to_string(index) + "/POWER";
}

std::string MqttTopics::availabilityTopic(const std::string& deviceName) noexcept {
    return "tele/irrigation/" + deviceName + "/LWT";
}

std::string MqttTopics::discoveryTopic(
    const std::string& nodeId,
    const std::string& deviceName,
    std::size_t index
) noexcept {
    return "homeassistant/switch/" + nodeId + "/" + deviceName + "_" + std::to_string(index) + "/config";
}

std::string MqttTopics::buildDiscoveryPayload(
    const std::string& nodeId,
    const std::string& deviceName,
    std::size_t index
) noexcept {
    const std::string idx = std::to_string(index);
    const std::string stateT = stateTopic(deviceName, index);
    const std::string cmndT = commandTopic(deviceName, index);
    const std::string availT = availabilityTopic(deviceName);

    std::string payload;
    payload.reserve(512);
    payload += "{";
    payload += "\"name\":\"" + deviceName + " Valve " + idx + "\",";
    payload += "\"unique_id\":\"" + nodeId + "_" + deviceName + "_" + idx + "\",";
    payload += "\"state_topic\":\"" + stateT + "\",";
    payload += "\"command_topic\":\"" + cmndT + "\",";
    payload += "\"payload_on\":\"";
    payload += PAYLOAD_ON;
    payload += "\",";
    payload += "\"payload_off\":\"";
    payload += PAYLOAD_OFF;
    payload += "\",";
    payload += "\"state_on\":\"";
    payload += PAYLOAD_ON;
    payload += "\",";
    payload += "\"state_off\":\"";
    payload += PAYLOAD_OFF;
    payload += "\",";
    payload += "\"availability_topic\":\"" + availT + "\",";
    payload += "\"payload_available\":\"Online\",";
    payload += "\"payload_not_available\":\"Offline\",";
    payload += "\"device\":{";
    payload += "\"identifiers\":[\"" + nodeId + "_" + deviceName + "\"],";
    payload += "\"name\":\"" + deviceName + "\",";
    payload += "\"manufacturer\":\"Leon Etienne\",";
    payload += "\"model\":\"ESP32 Irrigation Controller\"";
    payload += "}";
    payload += "}";

    return payload;
}

std::optional<std::size_t> MqttTopics::parseCommandTopic(
    const std::string& deviceName,
    std::string_view topic
) noexcept {
    static constexpr std::string_view prefixBase = "cmnd/irrigation/";
    static constexpr std::string_view suffix = "/POWER";

    const std::string prefix = std::string(prefixBase) + deviceName + "/";

    if (topic.size() <= prefix.size() + suffix.size()) {
        return std::nullopt;
    }
    if (topic.substr(0, prefix.size()) != prefix) {
        return std::nullopt;
    }
    if (topic.substr(topic.size() - suffix.size()) != suffix) {
        return std::nullopt;
    }

    const std::string_view indexStr = topic.substr(prefix.size(), topic.size() - prefix.size() - suffix.size());
    if (indexStr.empty()) {
        return std::nullopt;
    }

    std::size_t index = 0;
    for (const char c : indexStr) {
        if (c < '0' || c > '9') {
            return std::nullopt;
        }
        index = index * 10 + static_cast<std::size_t>(c - '0');
    }

    return index;
}

std::optional<bool> MqttTopics::parsePayload(std::string_view payload) noexcept {
    if (payload == PAYLOAD_ON) {
        return true;
    }
    if (payload == PAYLOAD_OFF) {
        return false;
    }
    return std::nullopt;
}
