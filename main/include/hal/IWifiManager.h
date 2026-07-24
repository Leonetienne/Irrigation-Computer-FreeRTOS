#ifndef IRRIGATION_COMPUTER_TESTS_IWIFIMANAGERS_H
#define IRRIGATION_COMPUTER_TESTS_IWIFIMANAGERS_H

#include <functional>
#include "enum/WifiConnectionState.h"
#include "WifiCredentials.h"

/**
 * Abstract interface to manage a wifi connection
 */
class IWifiManager {
public:
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
};

#endif //IRRIGATION_COMPUTER_TESTS_IWIFIMANAGERS_H
