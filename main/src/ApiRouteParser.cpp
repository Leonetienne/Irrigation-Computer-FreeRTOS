#include "ApiRouteParser.h"
#include <string>

std::optional<ValveCommand> ApiRouteParser::parseValveRoute(std::string_view uri) noexcept {
    std::string path(uri);

    const std::string prefix = "/api/valve/";
    if (path.rfind(prefix, 0) != 0) {
        return std::nullopt;
    }

    std::string rest = path.substr(prefix.length());

    size_t slashPos = rest.find('/');
    if (slashPos == std::string::npos) {
        return std::nullopt;
    }

    std::string indexStr = rest.substr(0, slashPos);
    std::string actionStr = rest.substr(slashPos + 1);

    if (indexStr.empty()) {
        return std::nullopt;
    }

    // manual atoi, no exceptions on embedded
    size_t valveIndex = 0;
    for (char c : indexStr) {
        if (c < '0' || c > '9') {
            return std::nullopt;
        }
        valveIndex = valveIndex * 10 + (c - '0');
    }

    ValveAction action;
    if (actionStr == "open") {
        action = ValveAction::Open;
    } else if (actionStr == "close") {
        action = ValveAction::Close;
    } else {
        return std::nullopt;
    }

    ValveCommand cmd{};
    cmd.valveIndex = valveIndex;
    cmd.action = action;
    return cmd;
}
