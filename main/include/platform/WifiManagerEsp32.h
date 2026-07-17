#ifndef IRRIGATION_COMPUTER_TESTS_WIFIMANAGERESP32_H
#define IRRIGATION_COMPUTER_TESTS_WIFIMANAGERESP32_H

#include "hal/IWifiManager.h"
#include "esp_event.h"
#include "esp_netif_types.h"

class WifiManagerEsp32 : public IWifiManager {
public:
    WifiManagerEsp32() = default;
    ~WifiManagerEsp32() noexcept override;

    WifiManagerEsp32(const WifiManagerEsp32&) = delete;
    WifiManagerEsp32& operator=(const WifiManagerEsp32&) = delete;
    WifiManagerEsp32(WifiManagerEsp32&&) = delete;
    WifiManagerEsp32& operator=(WifiManagerEsp32&&) = delete;

    /**
     * Initializes the wifi driver, registers the event handlers and
     * starts connecting to the given access point. Non-blocking; the
     * actual connection result arrives asynchronously via events.
     * @param ssid access point ssid
     * @param password access point password
     * @return Success state
     */
    bool begin(const char* ssid, const char* password) noexcept override;

    /**
     * Stops the wifi session and releases the resources acquired by begin()
     * @return Success state
     */
    bool free() noexcept override;

    /**
     * @return the current connection state
     */
    [[nodiscard]] WifiConnectionState getState() const noexcept override;

    /**
     * Registers a callback that gets invoked once a connection is
     * established (IP obtained).
     * @param callback function to call on connect
     */
    void setOnConnected(std::function<void()> callback) noexcept override;

    /**
     * Registers a callback that gets invoked once a previously
     * established connection is lost.
     * @param callback function to call on disconnect
     */
    void setOnDisconnected(std::function<void()> callback) noexcept override;

private:
    /**
     * Static esp-idf event callback. Routes back to the instance via
     * the "arg" pointer, updates connection state and fires the
     * registered callbacks.
     * @param arg instance pointer (this), passed on registration
     * @param base event base (WIFI_EVENT or IP_EVENT)
     * @param id event id
     * @param data event-specific data
     */
    static void eventHandler(void* arg, esp_event_base_t base, int32_t id, void* data) noexcept;

    bool isInitialized = false;
    WifiConnectionState state = WifiConnectionState::Disconnected;
    std::function<void()> onConnected;
    std::function<void()> onDisconnected;
    esp_netif_t* staNetif = nullptr;
};


#endif //IRRIGATION_COMPUTER_TESTS_WIFIMANAGERESP32_H
