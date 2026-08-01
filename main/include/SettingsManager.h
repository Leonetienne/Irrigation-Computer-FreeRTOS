#ifndef IRRIGATION_COMPUTER_SETTINGSMANAGER_H
#define IRRIGATION_COMPUTER_SETTINGSMANAGER_H

#include "hal/INVS.h"
#include "compat/gpio_num_t.h"
#include "WifiCredentials.h"
#include <array>
#include <expected>

/**
 * Class to interface with NVS to store and retrieve settings as well as defaults
 */
class SettingsManager {
public:
    SettingsManager(INVS& i_nvs) noexcept;
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager(SettingsManager&&) noexcept;
    SettingsManager& operator=(const SettingsManager&) = delete;
    SettingsManager& operator=(SettingsManager&&) = delete;

    /**
     * @return Success state
     */
    bool storeWifiCredentials(const WifiCredentials& wifiCredentials) const noexcept;
    [[nodiscard]] std::expected<WifiCredentials, bool> retrieveWifiCredentials() const noexcept;

    /**
     * @return Success state
     */
    bool storeTitle(const std::string& title) const noexcept;
    [[nodiscard]] std::expected<std::string, bool> retrieveTitle() const noexcept;

    /**
     * @return Success state
     */
    bool storeNumValves(const int32_t numValves) const noexcept;
    [[nodiscard]] std::expected<int32_t, bool> retrieveNumValves() const noexcept;

    /**
     * @param maxValveRuntime max valve runtime in minutes before it auto shuts
     * @return Success state
     */
    bool storeMaxValveRuntime(int32_t maxValveRuntime) const noexcept;
    [[nodiscard]] std::expected<int32_t, bool> retrieveMaxValveRuntime() const noexcept;

    /**
     * Stores which pin actuates which valve, packed into 2 int32 nvs entries (4 pins/int32)
     * @param gpioPins pin for valve 0..7
     * @return Success state
     */
    bool storeValveActuatorGpioPins(const std::array<gpio_num_t, 8>& gpioPins) const noexcept;

    /**
     * @return pin for valve 0..7, or false
     */
    [[nodiscard]] std::expected<std::array<gpio_num_t, 8>, bool> retrieveValveActuatorGpioPins() const noexcept;

private:
    INVS& i_nvs;
};


#endif //IRRIGATION_COMPUTER_SETTINGSMANAGER_H
