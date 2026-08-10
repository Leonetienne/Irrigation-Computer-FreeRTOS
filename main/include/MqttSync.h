#ifndef IRRIGATION_COMPUTER_TESTS_MQTTSYNC_H
#define IRRIGATION_COMPUTER_TESTS_MQTTSYNC_H

#include <array>
#include <optional>
#include <string>
#include "hal/IMqtt.h"
#include "ValveGroup.h"
#include "SettingsManager.h"

/**
 * Syncs a ValveGroup against an mqtt broker
 */
class MqttSync {
public:
    MqttSync(IMqtt& mqtt, ValveGroup& valveGroup, const SettingsManager& settings) noexcept;
    MqttSync(const MqttSync&) = delete;
    MqttSync& operator=(const MqttSync&) = delete;
    MqttSync(MqttSync&&) = delete;
    MqttSync& operator=(MqttSync&&) = delete;

    /**
     * Wires up the mqtt callbacks. Call once, before connect().
     */
    void begin() noexcept;

    /**
     * Connects to the broker configured in settings.
     * @return Success state. Fails if already connected or no broker uri is configured.
     */
    bool connect() noexcept;

    /**
     * Disconnects from the broker, if connected.
     * @return Success state
     */
    bool disconnect() noexcept;

    /**
     * Publishes the state of any valve whose open/closed state has changed
     * since the last call (or since connecting). Call repeatedly from the
     * runtime loop.
     */
    void pollPublishStateChanges() noexcept;

private:
    void onMqttConnected() noexcept;
    void onMqttDisconnected() noexcept;
    void onMqttMessage(const std::string& topic, const std::string& payload) noexcept;

    IMqtt& mqtt;
    ValveGroup& valveGroup;
    const SettingsManager& settings;

    bool isStarted = false;
    std::string deviceName;
    std::string nodeId;
    std::array<std::optional<bool>, 8> lastPublishedState{};
};

#endif //IRRIGATION_COMPUTER_TESTS_MQTTSYNC_H
