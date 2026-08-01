#include "test/stubs/WifiManagerStub.h"

WifiManagerStub::WifiManagerStub(WifiManagerStub&& other) noexcept :
    state(other.state),
    onConnected(std::move(other.onConnected)),
    onDisconnected(std::move(other.onDisconnected)),
    lastSsid(std::move(other.lastSsid)),
    lastPassword(std::move(other.lastPassword)),
    beginUserWifiCallCount(other.beginUserWifiCallCount),
    beginOnboardingWifiCallCount(other.beginOnboardingWifiCallCount)
{
    other.state = WifiConnectionState::Disconnected;
    other.beginUserWifiCallCount = 0;
    other.beginOnboardingWifiCallCount = 0;
}

bool WifiManagerStub::beginUserWifi(const WifiCredentials& credentials) noexcept {
    lastSsid = credentials.ssid;
    lastPassword = credentials.password;
    ++beginUserWifiCallCount;
    return true;
}

bool WifiManagerStub::beginOnboardingWifi() noexcept {
    ++beginOnboardingWifiCallCount;
    return true;
}

bool WifiManagerStub::free() noexcept {
    return true;
}

WifiConnectionState WifiManagerStub::getState() const noexcept {
    return state;
}

void WifiManagerStub::setOnConnected(std::function<void()> callback) noexcept {
    onConnected = std::move(callback);
}

void WifiManagerStub::setOnDisconnected(std::function<void()> callback) noexcept {
    onDisconnected = std::move(callback);
}

void WifiManagerStub::simulateConnected() {
    state = WifiConnectionState::Connected;
    if (onConnected) onConnected();
}

void WifiManagerStub::simulateDisconnected() {
    state = WifiConnectionState::Disconnected;
    if (onDisconnected) onDisconnected();
}

void WifiManagerStub::forceState(WifiConnectionState forcedState) {
    state = forcedState;
}

const std::string & WifiManagerStub::getLastSsid() const {
    return lastSsid;
}

const std::string & WifiManagerStub::getLastPassword() const {
    return lastPassword;
}

int WifiManagerStub::getBeginUserWifiCallCount() const {
    return beginUserWifiCallCount;
}

int WifiManagerStub::getBeginOnboardingWifiCallCount() const {
    return beginOnboardingWifiCallCount;
}
