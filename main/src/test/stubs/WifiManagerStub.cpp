#include "test/stubs/WifiManagerStub.h"

WifiManagerStub::WifiManagerStub(IGpio& gpio, GpioPinRegister& pinRegister, const ITime& i_time) noexcept :
    gpio(gpio),
    pinRegister(pinRegister),
    i_time(i_time)
{ }

WifiManagerStub::WifiManagerStub(WifiManagerStub&& other) noexcept :
    gpio(other.gpio),
    pinRegister(other.pinRegister),
    i_time(other.i_time),
    state(other.state),
    onConnected(std::move(other.onConnected)),
    onDisconnected(std::move(other.onDisconnected)),
    onFailed(std::move(other.onFailed)),
    lastSsid(std::move(other.lastSsid)),
    lastPassword(std::move(other.lastPassword)),
    beginUserWifiCallCount(other.beginUserWifiCallCount),
    beginOnboardingWifiCallCount(other.beginOnboardingWifiCallCount),
    indicatorPin(std::move(other.indicatorPin)),
    lastBlinkToggleAtMs(other.lastBlinkToggleAtMs)
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
    setIndicatorState(PIN_STATE_DIGITAL::LOW);
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

void WifiManagerStub::setOnFailed(std::function<void()> callback) noexcept {
    onFailed = std::move(callback);
}

bool WifiManagerStub::setIndicatorGpioPin(gpio_num_t pin) noexcept {
    if (indicatorPin.has_value()) {
        return false;
    }
    if (pin == GPIO_NUM_NC) {
        return true;
    }

    indicatorPin.emplace(pinRegister, gpio, pin);
    if (!indicatorPin->initialize()) {
        indicatorPin.reset();
        return false;
    }

    indicatorPin->setState(PIN_STATE_DIGITAL::LOW);
    lastBlinkToggleAtMs = i_time.getMillis();
    return true;
}

void WifiManagerStub::updateOnboardingModeLedBlink() noexcept {
    if (!indicatorPin.has_value() || !indicatorPin->isReady()) {
        return;
    }

    const int64_t now = i_time.getMillis();
    if (now - lastBlinkToggleAtMs < BLINK_INTERVAL_MS) {
        return;
    }

    lastBlinkToggleAtMs = now;
    setIndicatorState(
        indicatorPin->getState() == PIN_STATE_DIGITAL::HIGH ? PIN_STATE_DIGITAL::LOW : PIN_STATE_DIGITAL::HIGH
    );
}

void WifiManagerStub::setIndicatorState(PIN_STATE_DIGITAL pinState) noexcept {
    if (indicatorPin.has_value()) {
        indicatorPin->setState(pinState);
    }
}

void WifiManagerStub::simulateConnected() {
    state = WifiConnectionState::Connected;
    setIndicatorState(PIN_STATE_DIGITAL::HIGH);
    if (onConnected) onConnected();
}

void WifiManagerStub::simulateDisconnected() {
    state = WifiConnectionState::Disconnected;
    setIndicatorState(PIN_STATE_DIGITAL::LOW);
    if (onDisconnected) onDisconnected();
}

void WifiManagerStub::simulateFailed() {
    state = WifiConnectionState::Failed;
    setIndicatorState(PIN_STATE_DIGITAL::LOW);
    if (onFailed) onFailed();
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
