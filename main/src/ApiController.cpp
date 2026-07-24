#include "ApiController.h"

bool ApiController::executeValveOperation(ValveGroup& valveGroup, const ValveCommand& command) noexcept {
    switch (command.action) {
        case ValveAction::Open:
            return valveGroup.open(command.valveIndex);

        case ValveAction::Close:
            return valveGroup.close(command.valveIndex);
    }

    return false;
}

bool ApiController::saveWifiCredentials(
    INVS& nvs,
    StateMachine& stateMachine,
    const WifiCredentials& credentials
) noexcept {
    const bool saved =
        nvs.setString("wifi_ssid", credentials.ssid.c_str()) &&
        nvs.setString("wifi_pass", credentials.password.c_str());

    if (!saved) {
        return false;
    }

    stateMachine.setState(STATE::SHUTTING_DOWN);
    return true;
}
