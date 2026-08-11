#include "MqttSync.h"
#include "MqttTopics.h"
#include "compat/esp_log_macros.h"

static const char* LOG_TAG = "MqttSync";

MqttSync::MqttSync(IMqtt& mqtt, ValveGroup& valveGroup, const SettingsManager& settings) noexcept :
    mqtt(mqtt),
    valveGroup(valveGroup),
    settings(settings)
{ }

void MqttSync::begin() noexcept {
    mqtt.setOnConnected([this]() { onMqttConnected(); });
    mqtt.setOnDisconnected([this]() { onMqttDisconnected(); });
    mqtt.setOnMessage([this](const std::string& topic, const std::string& payload) {
        onMqttMessage(topic, payload);
    });
}

bool MqttSync::connect() noexcept {
    if (isStarted) {
        return false;
    }

    const auto brokerConfig = settings.retrieveMqttBrokerConfig();
    if (!brokerConfig.has_value() || brokerConfig->uri.empty()) {
        ESP_LOGI(LOG_TAG, "no broker uri configured, skipping mqtt connect");
        return false;
    }

    deviceName = settings.retrieveTitle().value_or("irrigation_computer");
    nodeId = settings.retrieveMqttNodeId().value_or("");
    if (nodeId.empty()) {
        nodeId = deviceName;
    }

    const MqttConnectOptions options{
        brokerConfig->uri,
        brokerConfig->username,
        brokerConfig->password,
        MqttTopics::availabilityTopic(deviceName),
        "Offline"
    };

    if (!mqtt.begin(options)) {
        ESP_LOGW(LOG_TAG, "mqtt.begin() failed for broker '%s'", options.brokerUri.c_str());
        return false;
    }

    ESP_LOGI(
        LOG_TAG,
        "connecting to broker '%s' as device '%s' (node id '%s')",
        options.brokerUri.c_str(),
        deviceName.c_str(),
        nodeId.c_str()
    );

    isStarted = true;
    lastPublishedState.fill(std::nullopt);
    return true;
}

bool MqttSync::disconnect() noexcept {
    if (!isStarted) {
        return false;
    }

    const bool success = mqtt.free();
    isStarted = false;
    lastPublishedState.fill(std::nullopt);
    return success;
}

void MqttSync::pollPublishStateChanges() noexcept {
    if (!isStarted || mqtt.getState() != MqttConnectionState::Connected) {
        return;
    }

    for (std::size_t i = 0; i < lastPublishedState.size(); ++i) {
        if (!valveGroup.isValveOperable(i)) {
            continue;
        }

        const bool open = valveGroup.getValveOpenState(i).value_or(false);
        if (lastPublishedState[i].has_value() && *lastPublishedState[i] == open) {
            continue;
        }

        mqtt.publish(
            MqttTopics::stateTopic(deviceName, i),
            std::string(open ? MqttTopics::PAYLOAD_ON : MqttTopics::PAYLOAD_OFF),
            1,
            true
        );
        lastPublishedState[i] = open;
    }
}

void MqttSync::pollActivityLedPulse() noexcept {
    mqtt.updateActivityLedPulse();
}

void MqttSync::onMqttConnected() noexcept {
    ESP_LOGI(LOG_TAG, "mqtt connected, publishing availability and per-valve discovery");
    mqtt.publish(MqttTopics::availabilityTopic(deviceName), "Online", 1, true);

    std::size_t publishedCount = 0;
    for (std::size_t i = 0; i < lastPublishedState.size(); ++i) {
        if (!valveGroup.isValveOperable(i)) {
            continue;
        }

        mqtt.subscribe(MqttTopics::commandTopic(deviceName, i), 1);
        mqtt.publish(
            MqttTopics::discoveryTopic(nodeId, deviceName, i),
            MqttTopics::buildDiscoveryPayload(nodeId, deviceName, i),
            1,
            true
        );

        const bool open = valveGroup.getValveOpenState(i).value_or(false);
        mqtt.publish(
            MqttTopics::stateTopic(deviceName, i),
            std::string(open ? MqttTopics::PAYLOAD_ON : MqttTopics::PAYLOAD_OFF),
            1,
            true
        );
        lastPublishedState[i] = open;
        ++publishedCount;
    }

    ESP_LOGI(LOG_TAG, "published discovery for %d operable valve(s)", static_cast<int>(publishedCount));
}

void MqttSync::onMqttDisconnected() noexcept {
    ESP_LOGW(LOG_TAG, "mqtt disconnected");

    if (!settings.retrieveCutOnMqttLossEnabled().value_or(true)) {
        return;
    }

    for (std::size_t i = 0; i < lastPublishedState.size(); ++i) {
        if (valveGroup.isValveOperable(i) && valveGroup.getValveOpenState(i).value_or(false)) {
            valveGroup.close(i);
        }
    }
}

void MqttSync::onMqttMessage(const std::string& topic, const std::string& payload) noexcept {
    const auto index = MqttTopics::parseCommandTopic(deviceName, topic);
    if (!index.has_value() || !valveGroup.isValveOperable(*index)) {
        ESP_LOGW(LOG_TAG, "ignoring message on unrecognized topic '%s'", topic.c_str());
        return;
    }

    const auto desiredOpen = MqttTopics::parsePayload(payload);
    if (!desiredOpen.has_value()) {
        ESP_LOGW(LOG_TAG, "ignoring unrecognized payload '%s' on topic '%s'", payload.c_str(), topic.c_str());
        return;
    }

    valveGroup.setOpenState(*index, *desiredOpen);
}
