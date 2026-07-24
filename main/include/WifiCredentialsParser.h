#ifndef IRRIGATION_COMPUTER_TESTS_WIFICREDENTIALSPARSER_H
#define IRRIGATION_COMPUTER_TESTS_WIFICREDENTIALSPARSER_H

#include <optional>
#include <string_view>
#include "WifiCredentials.h"

/**
 * Parses a "ssid=...&password=..." form-urlencoded request body into
 * typed wifi credentials.
 */
class WifiCredentialsParser {
public:
    /**
     * Parses a form-urlencoded body containing "ssid" and "password" fields
     * @param body request body
     * @return the parsed credentials, or std::nullopt on failure
     */
    [[nodiscard]] static std::optional<WifiCredentials> parse(std::string_view body) noexcept;
};

#endif //IRRIGATION_COMPUTER_TESTS_WIFICREDENTIALSPARSER_H
