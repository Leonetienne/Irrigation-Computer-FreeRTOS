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
    SettingsManager& settings,
    StateMachine& stateMachine,
    const WifiCredentials& credentials
) noexcept {
    if (!settings.storeWifiCredentials(credentials)) {
        return false;
    }

    stateMachine.setState(STATE::SHUTTING_DOWN);
    return true;
}
