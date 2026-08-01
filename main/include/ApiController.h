#ifndef IRRIGATION_COMPUTER_TESTS_APICONTROLLER_H
#define IRRIGATION_COMPUTER_TESTS_APICONTROLLER_H

#include "ValveGroup.h"
#include "ApiRouteParser.h"
#include "SettingsManager.h"
#include "WifiCredentials.h"
#include "StateMachine.h"

/**
 * Executes parsed api commands against the application state.
 */
class ApiController {
public:
    /**
     * Executes a parsed valve command against the given valve group
     * @param valveGroup the valve group to operate on
     * @param command
     * @return Success state
     */
    [[nodiscard]] static bool executeValveOperation(ValveGroup& valveGroup, const ValveCommand& command) noexcept;

    /**
     * Persists wifi credentials and requests a shutdown
     * @param settings
     * @param stateMachine
     * @param credentials
     * @return Success state
     */
    [[nodiscard]] static bool saveWifiCredentials(
        SettingsManager& settings,
        StateMachine& stateMachine,
        const WifiCredentials& credentials
    ) noexcept;
};

#endif //IRRIGATION_COMPUTER_TESTS_APICONTROLLER_H
