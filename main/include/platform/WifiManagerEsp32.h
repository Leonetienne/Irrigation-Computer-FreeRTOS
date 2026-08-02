#ifndef IRRIGATION_COMPUTER_TESTS_WIFIMANAGERESP32_H
#define IRRIGATION_COMPUTER_TESTS_WIFIMANAGERESP32_H

#include "hal/IWifiManager.h"
#include "esp_event.h"
#include "esp_netif_types.h"

class WifiManagerEsp32 : public IWifiManager {
public:
    WifiManagerEsp32() = default;
    WifiManagerEsp32(const WifiManagerEsp32&) = delete;
    WifiManagerEsp32& operator=(const WifiManagerEsp32&) = delete;
    WifiManagerEsp32(WifiManagerEsp32&&) = delete;
    WifiManagerEsp32& operator=(WifiManagerEsp32&&) = delete;
    ~WifiManagerEsp32() noexcept override;

    /**
     * Connects to an access point using the given credentials
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
     * Stops the wifi session and releases the resources acquired by
     * beginUserWifi()/beginOnboardingWifi()
     * @return Success state
     */
    bool free() noexcept override;

    /**
     * @return the current connection state
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
     * Callback setter. Fired once connecting has repeatedly failed and no further
     * automatic retries will be made.
     * @param callback
     */
    void setOnFailed(std::function<void()> callback) noexcept override;

private:
    /**
     * Static esp-idf event callback
     */
    static void eventHandler(void* arg, esp_event_base_t base, int32_t id, void* data) noexcept;

    static constexpr int MAX_CONNECT_RETRIES = 5;

    bool isInitialized = false;
    bool eventHandlersRegistered = false;
    WifiConnectionState state = WifiConnectionState::Disconnected;
    int connectFailureCount = 0;
    std::function<void()> onConnected;
    std::function<void()> onDisconnected;
    std::function<void()> onFailed;
    esp_netif_t* netif = nullptr;
};


#endif //IRRIGATION_COMPUTER_TESTS_WIFIMANAGERESP32_H
