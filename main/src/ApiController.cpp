#include "ApiController.h"
#include <array>

namespace {

// manual atoi, no exceptions on embedded
bool parseInt32(std::string_view str, int32_t& out) noexcept {
    if (str.empty()) {
        return false;
    }

    int32_t value = 0;
    for (const char c : str) {
        if (c < '0' || c > '9') {
            return false;
        }
        value = value * 10 + (c - '0');
    }

    out = value;
    return true;
}

// blank means "not connected", matching the GPIO_NUM_NC sentinel
gpio_num_t parseGpioField(const std::unordered_map<std::string, std::string>& form, const std::string& key) noexcept {
    const auto it = form.find(key);
    if (it == form.end() || it->second.empty()) {
        return GPIO_NUM_NC;
    }

    int32_t value = 0;
    if (!parseInt32(it->second, value)) {
        return GPIO_NUM_NC;
    }

    return static_cast<gpio_num_t>(value);
}

void appendGpioField(std::string& report, const std::string& key, gpio_num_t pin) noexcept {
    report += key;
    report += '=';
    if (pin != GPIO_NUM_NC) {
        report += std::to_string(static_cast<int32_t>(pin));
    }
    report += '\n';
}

std::string formValue(const std::unordered_map<std::string, std::string>& form, const std::string& key) noexcept {
    const auto it = form.find(key);
    return it != form.end() ? it->second : std::string();
}

}

ValveOperationResult ApiController::executeValveOperation(
    ValveGroup& valveGroup,
    const ValveCommand& command
) noexcept {
    if (!valveGroup.isValveOperable(command.valveIndex)) {
        return ValveOperationResult::InvalidRequest;
    }

    bool success = false;
    switch (command.action) {
        case ValveAction::Open:
            success = valveGroup.open(command.valveIndex);
            break;

        case ValveAction::Close:
            success = valveGroup.close(command.valveIndex);
            break;
    }

    return success ? ValveOperationResult::Success : ValveOperationResult::HardwareFailure;
}

bool ApiController::saveWifiCredentials(
    SettingsManager& settings,
    StateMachine& stateMachine,
    const WifiCredentials& credentials
) noexcept {
    WifiCredentials toStore = credentials;

    // a blank password means "keep the current one", unless there isn't one to keep
    if (toStore.password.empty()) {
        if (const auto existing = settings.retrieveWifiCredentials(); existing.has_value()) {
            toStore.password = existing->password;
        }
    }

    if (!settings.storeWifiCredentials(toStore)) {
        return false;
    }

    stateMachine.setState(STATE::SHUTTING_DOWN);
    return true;
}

std::string ApiController::buildValveStatusReport(
    const ValveGroup& valveGroup,
    const SettingsManager& settings
) noexcept {
    std::string report;
    const int32_t numValves = settings.retrieveNumValves().value_or(0);

    for (int32_t i = 0; i < numValves && i < 8; ++i) {
        report += std::to_string(i);
        report += ':';
        report += valveGroup.getValveOpenState(static_cast<std::size_t>(i)).value_or(false) ? '1' : '0';
        report += '\n';
    }

    return report;
}

std::string ApiController::buildValveConfigReport(const SettingsManager& settings) noexcept {
    std::string report;
    const int32_t numValves = settings.retrieveNumValves().value_or(0);

    report += "num_valves=";
    report += std::to_string(numValves);
    report += '\n';

    const auto pins = settings.retrieveValveActuatorGpioPins();
    for (int32_t i = 0; i < numValves && i < 8; ++i) {
        const gpio_num_t pin = pins.has_value() ? (*pins)[static_cast<std::size_t>(i)] : GPIO_NUM_NC;
        appendGpioField(report, "gpio" + std::to_string(i), pin);
    }

    return report;
}

std::string ApiController::buildSettingsReport(const SettingsManager& settings) noexcept {
    std::string report;

    report += "device_name=";
    if (const auto title = settings.retrieveTitle(); title.has_value()) {
        report += *title;
    }
    report += '\n';

    report += "max_valve_runtime_min=";
    if (const auto maxRuntime = settings.retrieveMaxValveRuntime(); maxRuntime.has_value()) {
        report += std::to_string(*maxRuntime);
    }
    report += '\n';

    const auto wifiCredentials = settings.retrieveWifiCredentials();

    report += "wifi_ssid=";
    if (wifiCredentials.has_value()) {
        report += wifiCredentials->ssid;
    }
    report += '\n';

    report += "wifi_password_set=";
    report += (wifiCredentials.has_value() && !wifiCredentials->password.empty()) ? '1' : '0';
    report += '\n';

    report += "runtime_safety_enabled=";
    if (const auto runtimeSafety = settings.retrieveRuntimeSafetyEnabled(); runtimeSafety.has_value()) {
        report += *runtimeSafety ? '1' : '0';
    }
    report += '\n';

    report += "cut_on_wifi_loss_enabled=";
    if (const auto cutOnWifiLoss = settings.retrieveCutOnWifiLossEnabled(); cutOnWifiLoss.has_value()) {
        report += *cutOnWifiLoss ? '1' : '0';
    }
    report += '\n';

    const auto mqttConfig = settings.retrieveMqttBrokerConfig();

    report += "mqtt_uri=";
    if (mqttConfig.has_value()) {
        report += mqttConfig->uri;
    }
    report += '\n';

    report += "mqtt_user=";
    if (mqttConfig.has_value()) {
        report += mqttConfig->username;
    }
    report += '\n';

    report += "mqtt_password_set=";
    report += (mqttConfig.has_value() && !mqttConfig->password.empty()) ? '1' : '0';
    report += '\n';

    report += "node_id=";
    if (const auto nodeId = settings.retrieveMqttNodeId(); nodeId.has_value()) {
        report += *nodeId;
    }
    report += '\n';

    report += "cut_on_mqtt_loss_enabled=";
    if (const auto cutOnMqttLoss = settings.retrieveCutOnMqttLossEnabled(); cutOnMqttLoss.has_value()) {
        report += *cutOnMqttLoss ? '1' : '0';
    }
    report += '\n';

    return report;
}

bool ApiController::applySettingsForm(
    SettingsManager& settings,
    StateMachine& stateMachine,
    const std::unordered_map<std::string, std::string>& form
) noexcept {
    const auto nameIt = form.find("device_name");
    if (nameIt == form.end() || nameIt->second.empty()) {
        return false;
    }

    int32_t maxValveRuntime = 0;
    const auto runtimeIt = form.find("max_valve_runtime_min");
    if (runtimeIt == form.end() || !parseInt32(runtimeIt->second, maxValveRuntime)) {
        return false;
    }

    const bool runtimeSafetyEnabled = form.contains("enable_runtime_safety");
    const bool cutOnWifiLossEnabled = form.contains("enable_cut_on_wifi_loss");
    const bool cutOnMqttLossEnabled = form.contains("enable_cut_on_mqtt_loss");

    MqttBrokerConfig mqttConfig{
        formValue(form, "mqtt_uri"),
        formValue(form, "mqtt_user"),
        formValue(form, "mqtt_pass")
    };

    // a blank password means "keep the current one", unless there isn't one to keep
    if (mqttConfig.password.empty()) {
        if (const auto existing = settings.retrieveMqttBrokerConfig(); existing.has_value()) {
            mqttConfig.password = existing->password;
        }
    }

    if (!settings.storeTitle(nameIt->second) ||
        !settings.storeMaxValveRuntime(maxValveRuntime) ||
        !settings.storeRuntimeSafetyEnabled(runtimeSafetyEnabled) ||
        !settings.storeCutOnWifiLossEnabled(cutOnWifiLossEnabled) ||
        !settings.storeMqttBrokerConfig(mqttConfig) ||
        !settings.storeMqttNodeId(formValue(form, "node_id")) ||
        !settings.storeCutOnMqttLossEnabled(cutOnMqttLossEnabled)) {
        return false;
    }

    stateMachine.setState(STATE::SHUTTING_DOWN);
    return true;
}

std::string ApiController::buildAdvancedSettingsReport(const SettingsManager& settings) noexcept {
    std::string report;

    report += "num_valves=";
    if (const auto numValves = settings.retrieveNumValves(); numValves.has_value()) {
        report += std::to_string(*numValves);
    }
    report += '\n';

    const auto pins = settings.retrieveValveActuatorGpioPins();
    for (std::size_t i = 0; i < 8; ++i) {
        const gpio_num_t pin = pins.has_value() ? (*pins)[i] : GPIO_NUM_NC;
        appendGpioField(report, "gpio" + std::to_string(i), pin);
    }

    return report;
}

bool ApiController::applyAdvancedSettingsForm(
    SettingsManager& settings,
    StateMachine& stateMachine,
    const std::unordered_map<std::string, std::string>& form
) noexcept {
    const auto numValvesIt = form.find("num_valves");
    int32_t numValves = 0;
    if (numValvesIt == form.end() || !parseInt32(numValvesIt->second, numValves)) {
        return false;
    }

    // pins beyond the configured valve count are unset, not just left disabled client-side
    std::array<gpio_num_t, 8> pins{};
    for (std::size_t i = 0; i < 8; ++i) {
        pins[i] = static_cast<int32_t>(i) < numValves
            ? parseGpioField(form, "gpio" + std::to_string(i))
            : GPIO_NUM_NC;
    }

    if (!settings.storeNumValves(numValves) || !settings.storeValveActuatorGpioPins(pins)) {
        return false;
    }

    stateMachine.setState(STATE::SHUTTING_DOWN);
    return true;
}
