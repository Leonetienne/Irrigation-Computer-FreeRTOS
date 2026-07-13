#include "test/stubs/WifiManagerStub.h"

void WifiManagerStub::begin(const char* ssid, const char* password) {
    lastSsid = ssid;
    lastPassword = password;
    ++beginCallCount;
}

void WifiManagerStub::setOnConnected(std::function<void()> callback) {
    onConnected = std::move(callback);
}

void WifiManagerStub::setOnDisconnected(std::function<void()> callback) {
    onDisconnected = std::move(callback);
}

void WifiManagerStub::simulateConnected() {
    state_ = WifiConnectionState::Connected;
    if (onConnected) onConnected();
}

void WifiManagerStub::simulateDisconnected() {
    state_ = WifiConnectionState::Disconnected;
    if (onDisconnected) onDisconnected();
}