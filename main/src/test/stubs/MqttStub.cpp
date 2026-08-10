#include "test/stubs/MqttStub.h"

bool MqttStub::begin(const MqttConnectOptions& options) noexcept {
    lastConnectOptions = options;
    ++beginCallCount;
    isInitialized = true;
    return true;
}

bool MqttStub::free() noexcept {
    state = MqttConnectionState::Disconnected;
    isInitialized = false;
    return true;
}

bool MqttStub::publish(const std::string& topic, const std::string& payload, int qos, bool retain) noexcept {
    publishedMessages.push_back({topic, payload, qos, retain});
    return true;
}

bool MqttStub::subscribe(const std::string& topic, int qos) noexcept {
    subscriptions.push_back({topic, qos});
    return true;
}

MqttConnectionState MqttStub::getState() const noexcept {
    return state;
}

void MqttStub::setOnConnected(std::function<void()> callback) noexcept {
    onConnected = std::move(callback);
}

void MqttStub::setOnDisconnected(std::function<void()> callback) noexcept {
    onDisconnected = std::move(callback);
}

void MqttStub::setOnMessage(std::function<void(const std::string& topic, const std::string& payload)> callback) noexcept {
    onMessage = std::move(callback);
}

void MqttStub::simulateConnected() {
    state = MqttConnectionState::Connected;
    if (onConnected) onConnected();
}

void MqttStub::simulateDisconnected() {
    state = MqttConnectionState::Disconnected;
    if (onDisconnected) onDisconnected();
}

void MqttStub::simulateMessage(const std::string& topic, const std::string& payload) {
    if (onMessage) onMessage(topic, payload);
}

const MqttConnectOptions& MqttStub::getLastConnectOptions() const {
    return lastConnectOptions;
}

int MqttStub::getBeginCallCount() const {
    return beginCallCount;
}

const std::vector<MqttPublishedMessage>& MqttStub::getPublishedMessages() const {
    return publishedMessages;
}

const std::vector<MqttSubscription>& MqttStub::getSubscriptions() const {
    return subscriptions;
}
