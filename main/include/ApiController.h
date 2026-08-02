#ifndef IRRIGATION_COMPUTER_TESTS_APICONTROLLER_H
#define IRRIGATION_COMPUTER_TESTS_APICONTROLLER_H

#include "ValveGroup.h"
#include "ApiRouteParser.h"
#include "SettingsManager.h"
#include "WifiCredentials.h"
#include "StateMachine.h"
#include <string>
#include <unordered_map>

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

    /**
     * @param valveGroup
     * @return "idx:state\n" lines for every valve, state being 1 (open) or 0 (closed)
     */
    [[nodiscard]] static std::string buildValveReport(const ValveGroup& valveGroup) noexcept;

    /**
     * @param settings
     * @return "key=value\n" lines describing the current settings
     */
    [[nodiscard]] static std::string buildSettingsReport(const SettingsManager& settings) noexcept;

    /**
     * Applies a parsed settings form and requests a shutdown
     * @param settings
     * @param stateMachine
     * @param form
     * @return Success state
     */
    [[nodiscard]] static bool applySettingsForm(
        SettingsManager& settings,
        StateMachine& stateMachine,
        const std::unordered_map<std::string, std::string>& form
    ) noexcept;

    /**
     * @param settings
     * @return "key=value\n" lines describing the current advanced settings
     */
    [[nodiscard]] static std::string buildAdvancedSettingsReport(const SettingsManager& settings) noexcept;

    /**
     * Applies a parsed advanced settings form and requests a shutdown
     * @param settings
     * @param stateMachine
     * @param form
     * @return Success state
     */
    [[nodiscard]] static bool applyAdvancedSettingsForm(
        SettingsManager& settings,
        StateMachine& stateMachine,
        const std::unordered_map<std::string, std::string>& form
    ) noexcept;
};

#endif //IRRIGATION_COMPUTER_TESTS_APICONTROLLER_H
