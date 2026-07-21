#ifndef IRRIGATION_COMPUTER_TESTS_APIROUTEPARSER_H
#define IRRIGATION_COMPUTER_TESTS_APIROUTEPARSER_H

#include <cstddef>
#include <optional>
#include <string_view>
#include "enum/ValveAction.h"

struct ValveCommand {
    std::size_t valveIndex;
    ValveAction action;
};

/**
 * Parses http api routes into typed commands
 */
class ApiRouteParser {
public:
    /**
     * Parses a route of the form "/api/valve/{index}/{open|close}"
     * @param uri request path
     * @return the parsed command, or std::nullopt if the route doesn't match
     */
    [[nodiscard]] static std::optional<ValveCommand> parseValveRoute(std::string_view uri) noexcept;
};

#endif //IRRIGATION_COMPUTER_TESTS_APIROUTEPARSER_H
