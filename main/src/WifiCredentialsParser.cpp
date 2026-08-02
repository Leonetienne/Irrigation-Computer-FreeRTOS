#include "WifiCredentialsParser.h"
#include "UrlEncodedForm.h"

std::optional<WifiCredentials> WifiCredentialsParser::parse(std::string_view body) noexcept {
    const auto values = UrlEncodedForm::parse(body);

    const auto ssidIt = values.find("ssid");
    if (ssidIt == values.end() || ssidIt->second.empty()) {
        return std::nullopt;
    }

    const auto passwordIt = values.find("password");
    if (passwordIt == values.end()) {
        return std::nullopt;
    }

    return WifiCredentials{ssidIt->second, passwordIt->second};
}
