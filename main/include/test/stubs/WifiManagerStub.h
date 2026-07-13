#ifndef IRRIGATION_COMPUTER_TESTS_WIFIMANAGERSTUB_H
#define IRRIGATION_COMPUTER_TESTS_WIFIMANAGERSTUB_H

#include "hal/IWifiManager.h"
#include <string>

class WifiManagerStub : public IWifiManager {
public:
    WifiManagerStub() = default;
    WifiManagerStub(const WifiManagerStub&) = delete;
    WifiManagerStub& operator=(const WifiManagerStub&) = delete;
    WifiManagerStub(WifiManagerStub&&) = delete;
    WifiManagerStub& operator=(WifiManagerStub&&) = delete;

    /**
     * Will begin the wifi session
     * @param ssid
     * @param password
     */
    void begin(const char* ssid, const char* password) override;

    /**
     * @return The wifi session state
     */
    [[nodiscard]] WifiConnectionState getState() const override { return state_; }

    /**
     * Callback setter
     * @param callback
     */
    void setOnConnected(std::function<void()> callback) override;
    /**
     * Callback setter
     * @param callback
     */
    void setOnDisconnected(std::function<void()> callback) override;

    /**
     * fires callbacks, simulates a real connect/disconnect event
     */
    void simulateConnected();
    void simulateDisconnected();

    /**
     *
     * @param state sets state directly, no callbacks fired
     */
    void forceState(WifiConnectionState state) { state_ = state; }

    [[nodiscard]] const std::string& getLastSsid() const { return lastSsid; }
    [[nodiscard]] const std::string& getLastPassword() const { return lastPassword; }
    [[nodiscard]] int getBeginCallCount() const { return beginCallCount; }

private:
    WifiConnectionState state_ = WifiConnectionState::Disconnected;
    std::function<void()> onConnected;
    std::function<void()> onDisconnected;

    std::string lastSsid;
    std::string lastPassword;
    int beginCallCount = 0;
};

#endif //IRRIGATION_COMPUTER_TESTS_WIFIMANAGERSTUB_H
