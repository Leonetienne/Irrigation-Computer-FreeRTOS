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
     * Connects to an existing access point using the given credentials
     * @param credentials
     * @return Success state
     */
    bool beginUserWifi(const WifiCredentials& credentials) noexcept override;

    /**
     * Spawns this device's own open access point for onboarding
     * @return Success state
     */
    bool beginOnboardingWifi() noexcept override;

    /**
     * Will release the resources acquired by beginUserWifi()/beginOnboardingWifi()
     * @return Success state
     */
    bool free() noexcept override;

    /**
     * @return The wifi session state
     */
    [[nodiscard]] WifiConnectionState getState() const noexcept override;

    /**
     * Callback setter
     * @param callback
     */
    void setOnConnected(std::function<void()> callback) noexcept override;
    /**
     * Callback setter
     * @param callback
     */
    void setOnDisconnected(std::function<void()> callback) noexcept override;

    /**
     * fires callbacks, simulates a real connect/disconnect event
     */
    void simulateConnected();
    void simulateDisconnected();

    /**
     *
     * @param forcedState sets state directly, no callbacks fired
     */
    void forceState(WifiConnectionState forcedState);

    [[nodiscard]] const std::string& getLastSsid() const;
    [[nodiscard]] const std::string& getLastPassword() const;
    [[nodiscard]] int getBeginUserWifiCallCount() const;
    [[nodiscard]] int getBeginOnboardingWifiCallCount() const;

private:
    WifiConnectionState state = WifiConnectionState::Disconnected;
    std::function<void()> onConnected;
    std::function<void()> onDisconnected;

    std::string lastSsid;
    std::string lastPassword;
    int beginUserWifiCallCount = 0;
    int beginOnboardingWifiCallCount = 0;
};

#endif //IRRIGATION_COMPUTER_TESTS_WIFIMANAGERSTUB_H
