#ifndef IRRIGATION_COMPUTER_TESTS_MQTTTOPICS_H
#define IRRIGATION_COMPUTER_TESTS_MQTTTOPICS_H

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

/**
 * Builder/parser for this device's mqtt topic scheme and Home Assistant discovery payloads
 */
class MqttTopics {
public:
    static constexpr std::string_view PAYLOAD_ON = "ON";
    static constexpr std::string_view PAYLOAD_OFF = "OFF";

    /**
     * @return "cmnd/irrigation/{deviceName}/{index}/POWER"
     */
    [[nodiscard]] static std::string commandTopic(const std::string& deviceName, std::size_t index) noexcept;

    /**
     * @return "stat/irrigation/{deviceName}/{index}/POWER"
     */
    [[nodiscard]] static std::string stateTopic(const std::string& deviceName, std::size_t index) noexcept;

    /**
     * @return "tele/irrigation/{deviceName}/LWT"
     */
    [[nodiscard]] static std::string availabilityTopic(const std::string& deviceName) noexcept;

    /**
     * @return "homeassistant/switch/{nodeId}/{deviceName}_{index}/config"
     */
    [[nodiscard]] static std::string discoveryTopic(
        const std::string& nodeId,
        const std::string& deviceName,
        std::size_t index
    ) noexcept;

    /**
     * @return the Home Assistant MQTT switch discovery payload for a single valve
     */
    [[nodiscard]] static std::string buildDiscoveryPayload(
        const std::string& nodeId,
        const std::string& deviceName,
        std::size_t index
    ) noexcept;

    /**
     * @param deviceName
     * @param topic
     * @return the valve index if topic matches this device's command topic pattern, else nullopt
     */
    [[nodiscard]] static std::optional<std::size_t> parseCommandTopic(
        const std::string& deviceName,
        std::string_view topic
    ) noexcept;

    /**
     * @param payload
     * @return true for "ON", false for "OFF", nullopt for anything else
     */
    [[nodiscard]] static std::optional<bool> parsePayload(std::string_view payload) noexcept;
};

#endif //IRRIGATION_COMPUTER_TESTS_MQTTTOPICS_H
