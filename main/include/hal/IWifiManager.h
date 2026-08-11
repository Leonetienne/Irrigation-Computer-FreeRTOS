#ifndef IRRIGATION_COMPUTER_TESTS_IWIFIMANAGERS_H
#define IRRIGATION_COMPUTER_TESTS_IWIFIMANAGERS_H

#include <functional>
#include "enum/WifiConnectionState.h"
#include "WifiCredentials.h"
#include "compat/gpio_num_t.h"

/**
 * Abstract interface to manage a wifi connection
 */
class IWifiManager {
public:
    IWifiManager() = default;
    IWifiManager(const IWifiManager&) = delete;
    IWifiManager& operator=(const IWifiManager&) = delete;
    IWifiManager(IWifiManager&&) = delete;
    IWifiManager& operator=(IWifiManager&&) = delete;
    virtual ~IWifiManager() = default;

    /**
     * Connects to an existing access point using the given credentials
     * @param credentials
     * @return Success state
     */
    virtual bool beginUserWifi(const WifiCredentials& credentials) noexcept = 0;

    /**
     * Spawns this device's own open access point for onboarding
     * @return Success state
     */
    virtual bool beginOnboardingWifi() noexcept = 0;

    /**
     * Will release the resources acquired by beginUserWifi()/beginOnboardingWifi()
     * @return Success state
     */
    virtual bool free() noexcept = 0;

    /**
     * @return The wifi session state
     */
    [[nodiscard]] virtual WifiConnectionState getState() const noexcept = 0;

    /**
     * Callback setter
     * @param callback
     */
    virtual void setOnConnected(std::function<void()> callback) noexcept = 0;
    /**
     * Callback setter
     * @param callback
     */
    virtual void setOnDisconnected(std::function<void()> callback) noexcept = 0;

    /**
     * Callback setter. Fired once connecting to the configured access point has
     * repeatedly failed (e.g. wrong credentials) and no further automatic retries
     * will be made.
     * @param callback
     */
    virtual void setOnFailed(std::function<void()> callback) noexcept = 0;

    /**
     * Configures the wifi status indicator LED pin. Pass GPIO_NUM_NC to leave
     * it unconfigured (all LED behavior becomes a no-op). Call once, before
     * beginUserWifi()/beginOnboardingWifi().
     * @return Success state
     */
    virtual bool setIndicatorGpioPin(gpio_num_t pin) noexcept = 0;

    /**
     * Advances the onboarding-mode LED blink (toggles every 500ms). The caller
     * is responsible for only polling this while the system is actually in
     * onboarding mode; a no-op if no indicator pin is configured.
     */
    virtual void updateOnboardingModeLedBlink() noexcept = 0;
};

#endif //IRRIGATION_COMPUTER_TESTS_IWIFIMANAGERS_H
