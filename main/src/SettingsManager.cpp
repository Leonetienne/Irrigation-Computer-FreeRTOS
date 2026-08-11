#include "../include/SettingsManager.h"

namespace {

// packs 4 gpio_num_t (each fits in a byte, -1..39) into one int32
int32_t packGpioPins(const gpio_num_t* pins) {
    uint32_t packed = 0;
    for (std::size_t i = 0; i < 4; ++i) {
        packed |= static_cast<uint32_t>(static_cast<uint8_t>(pins[i])) << (8 * i);
    }
    return static_cast<int32_t>(packed);
}

void unpackGpioPins(int32_t packed, gpio_num_t* outPins) {
    const auto word = static_cast<uint32_t>(packed);
    for (std::size_t i = 0; i < 4; ++i) {
        const auto byte = static_cast<uint8_t>(word >> (8 * i));
        outPins[i] = static_cast<gpio_num_t>(static_cast<int8_t>(byte));
    }
}

}

SettingsManager::SettingsManager(INVS &i_nvs) noexcept :
    i_nvs (i_nvs)
{ }

SettingsManager::SettingsManager(SettingsManager && other) noexcept :
    i_nvs(other.i_nvs)
{}

bool SettingsManager::storeWifiCredentials(const WifiCredentials& wifiCredentials) const noexcept {
    if (!i_nvs.isReady()) {
        return false;
    }
    const bool saved =
        i_nvs.setString("wifi_ssid", wifiCredentials.ssid.c_str()) &&
        i_nvs.setString("wifi_pass", wifiCredentials.password.c_str());

    if (!saved) {
        return false;
    }

    return true;
}

std::expected<WifiCredentials, bool> SettingsManager::retrieveWifiCredentials() const noexcept {
    char storedSsid[NVS_MAX_STRING_LENGTH + 1] = {};
    char storedPassword[NVS_MAX_STRING_LENGTH + 1] = {};
    const bool hasStoredCredentials =
        i_nvs.getString("wifi_ssid", storedSsid) &&
        i_nvs.getString("wifi_pass", storedPassword);

    if (!hasStoredCredentials) {
        return std::unexpected(false);
    }

    return WifiCredentials{storedSsid, storedPassword};
}

bool SettingsManager::eraseWifiCredentials() const noexcept {
    return
        i_nvs.eraseKey("wifi_ssid") &&
        i_nvs.eraseKey("wifi_pass");
}

bool SettingsManager::storeTitle(const std::string &title) const noexcept {
    return i_nvs.setString("title", title.c_str());
}

std::expected<std::string, bool> SettingsManager::retrieveTitle() const noexcept {
    char buf[NVS_MAX_STRING_LENGTH + 1] = {};
    if (!i_nvs.getString("title", buf)) {
        return std::unexpected(false);
    }
    return std::string(buf);
}

bool SettingsManager::storeNumValves(const int32_t numValves) const noexcept {
    return i_nvs.setInt("num_valves", numValves);
}

std::expected<int32_t, bool> SettingsManager::retrieveNumValves() const noexcept {
    if (int32_t buf{}; i_nvs.getInt("num_valves", buf)) {
        return buf;
    }
    return std::unexpected(false);
}

bool SettingsManager::storeMaxValveRuntime(int32_t maxValveRuntime) const noexcept {
    return i_nvs.setInt("max_vruntime", maxValveRuntime);
}

std::expected<int32_t, bool> SettingsManager::retrieveMaxValveRuntime() const noexcept {
    if (int32_t buf{}; i_nvs.getInt("max_vruntime", buf)) {
        return buf;
    }
    return std::unexpected(false);
}

bool SettingsManager::storeValveActuatorGpioPins(const std::array<gpio_num_t, 8>& gpioPins) const noexcept {
    return
        i_nvs.setInt("valve_pins_0", packGpioPins(gpioPins.data())) &&
        i_nvs.setInt("valve_pins_1", packGpioPins(gpioPins.data() + 4));
}

std::expected<std::array<gpio_num_t, 8>, bool> SettingsManager::retrieveValveActuatorGpioPins() const noexcept {
    int32_t word0{};
    int32_t word1{};
    const bool loaded =
        i_nvs.getInt("valve_pins_0", word0) &&
        i_nvs.getInt("valve_pins_1", word1);

    if (!loaded) {
        return std::unexpected(false);
    }

    std::array<gpio_num_t, 8> pins{};
    unpackGpioPins(word0, pins.data());
    unpackGpioPins(word1, pins.data() + 4);
    return pins;
}

bool SettingsManager::storeValveIndicatorGpioPins(const std::array<gpio_num_t, 8>& gpioPins) const noexcept {
    return
        i_nvs.setInt("valve_ind_0", packGpioPins(gpioPins.data())) &&
        i_nvs.setInt("valve_ind_1", packGpioPins(gpioPins.data() + 4));
}

std::expected<std::array<gpio_num_t, 8>, bool> SettingsManager::retrieveValveIndicatorGpioPins() const noexcept {
    int32_t word0{};
    int32_t word1{};
    const bool loaded =
        i_nvs.getInt("valve_ind_0", word0) &&
        i_nvs.getInt("valve_ind_1", word1);

    if (!loaded) {
        return std::unexpected(false);
    }

    std::array<gpio_num_t, 8> pins{};
    unpackGpioPins(word0, pins.data());
    unpackGpioPins(word1, pins.data() + 4);
    return pins;
}

bool SettingsManager::storeWifiLedGpioPin(gpio_num_t gpioPin) const noexcept {
    return i_nvs.setInt("wifi_led_gpio", static_cast<int32_t>(gpioPin));
}

std::expected<gpio_num_t, bool> SettingsManager::retrieveWifiLedGpioPin() const noexcept {
    if (int32_t buf{}; i_nvs.getInt("wifi_led_gpio", buf)) {
        return static_cast<gpio_num_t>(buf);
    }
    return std::unexpected(false);
}

bool SettingsManager::storeMqttLedGpioPin(gpio_num_t gpioPin) const noexcept {
    return i_nvs.setInt("mqtt_led_gpio", static_cast<int32_t>(gpioPin));
}

std::expected<gpio_num_t, bool> SettingsManager::retrieveMqttLedGpioPin() const noexcept {
    if (int32_t buf{}; i_nvs.getInt("mqtt_led_gpio", buf)) {
        return static_cast<gpio_num_t>(buf);
    }
    return std::unexpected(false);
}

bool SettingsManager::storeConnLedsEnabled(bool enabled) const noexcept {
    return i_nvs.setInt("conn_leds_en", enabled ? 1 : 0);
}

std::expected<bool, bool> SettingsManager::retrieveConnLedsEnabled() const noexcept {
    if (int32_t buf{}; i_nvs.getInt("conn_leds_en", buf)) {
        return buf != 0;
    }
    return std::unexpected(false);
}

bool SettingsManager::storeValveLedsEnabled(bool enabled) const noexcept {
    return i_nvs.setInt("valve_leds_en", enabled ? 1 : 0);
}

std::expected<bool, bool> SettingsManager::retrieveValveLedsEnabled() const noexcept {
    if (int32_t buf{}; i_nvs.getInt("valve_leds_en", buf)) {
        return buf != 0;
    }
    return std::unexpected(false);
}

bool SettingsManager::storeRuntimeSafetyEnabled(bool enabled) const noexcept {
    return i_nvs.setInt("rt_safety_en", enabled ? 1 : 0);
}

std::expected<bool, bool> SettingsManager::retrieveRuntimeSafetyEnabled() const noexcept {
    if (int32_t buf{}; i_nvs.getInt("rt_safety_en", buf)) {
        return buf != 0;
    }
    return std::unexpected(false);
}

bool SettingsManager::storeCutOnWifiLossEnabled(bool enabled) const noexcept {
    return i_nvs.setInt("cut_wifi_en", enabled ? 1 : 0);
}

std::expected<bool, bool> SettingsManager::retrieveCutOnWifiLossEnabled() const noexcept {
    if (int32_t buf{}; i_nvs.getInt("cut_wifi_en", buf)) {
        return buf != 0;
    }
    return std::unexpected(false);
}

bool SettingsManager::storeMqttBrokerConfig(const MqttBrokerConfig& config) const noexcept {
    return
        i_nvs.setString("mqtt_uri", config.uri.c_str()) &&
        i_nvs.setString("mqtt_user", config.username.c_str()) &&
        i_nvs.setString("mqtt_pass", config.password.c_str());
}

std::expected<MqttBrokerConfig, bool> SettingsManager::retrieveMqttBrokerConfig() const noexcept {
    char uri[NVS_MAX_STRING_LENGTH + 1] = {};
    if (!i_nvs.getString("mqtt_uri", uri)) {
        return std::unexpected(false);
    }

    char user[NVS_MAX_STRING_LENGTH + 1] = {};
    char pass[NVS_MAX_STRING_LENGTH + 1] = {};
    i_nvs.getString("mqtt_user", user);
    i_nvs.getString("mqtt_pass", pass);

    return MqttBrokerConfig{uri, user, pass};
}

bool SettingsManager::storeMqttNodeId(const std::string& nodeId) const noexcept {
    return i_nvs.setString("node_id", nodeId.c_str());
}

std::expected<std::string, bool> SettingsManager::retrieveMqttNodeId() const noexcept {
    char buf[NVS_MAX_STRING_LENGTH + 1] = {};
    if (!i_nvs.getString("node_id", buf)) {
        return std::unexpected(false);
    }
    return std::string(buf);
}

bool SettingsManager::storeCutOnMqttLossEnabled(bool enabled) const noexcept {
    return i_nvs.setInt("cut_mqtt_en", enabled ? 1 : 0);
}

std::expected<bool, bool> SettingsManager::retrieveCutOnMqttLossEnabled() const noexcept {
    if (int32_t buf{}; i_nvs.getInt("cut_mqtt_en", buf)) {
        return buf != 0;
    }
    return std::unexpected(false);
}
