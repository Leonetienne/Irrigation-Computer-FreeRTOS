#include <catch2/catch_test_macros.hpp>
#include "ApiRouteParser.h"

TEST_CASE("ApiRouteParser: parseValveRoute", "[ApiRouteParser]") {
    SECTION("parses an open route") {
        const auto command = ApiRouteParser::parseValveRoute("/api/valve/0/open");
        REQUIRE(command.has_value());
        REQUIRE(command->valveIndex == 0);
        REQUIRE(command->action == ValveAction::Open);
    }

    SECTION("parses a close route") {
        const auto command = ApiRouteParser::parseValveRoute("/api/valve/3/close");
        REQUIRE(command.has_value());
        REQUIRE(command->valveIndex == 3);
        REQUIRE(command->action == ValveAction::Close);
    }

    SECTION("parses multi-digit indices") {
        const auto command = ApiRouteParser::parseValveRoute("/api/valve/42/open");
        REQUIRE(command.has_value());
        REQUIRE(command->valveIndex == 42);
    }

    SECTION("fails for an unknown action") {
        REQUIRE_FALSE(ApiRouteParser::parseValveRoute("/api/valve/0/explode").has_value());
    }

    SECTION("fails for toggle - the api only supports open/close") {
        REQUIRE_FALSE(ApiRouteParser::parseValveRoute("/api/valve/0/toggle").has_value());
    }

    SECTION("fails for a non-numeric index") {
        REQUIRE_FALSE(ApiRouteParser::parseValveRoute("/api/valve/abc/open").has_value());
    }

    SECTION("fails for a negative index") {
        REQUIRE_FALSE(ApiRouteParser::parseValveRoute("/api/valve/-1/open").has_value());
    }

    SECTION("fails when the action is missing") {
        REQUIRE_FALSE(ApiRouteParser::parseValveRoute("/api/valve/0").has_value());
        REQUIRE_FALSE(ApiRouteParser::parseValveRoute("/api/valve/0/").has_value());
    }

    SECTION("fails when the index is missing") {
        REQUIRE_FALSE(ApiRouteParser::parseValveRoute("/api/valve//open").has_value());
    }

    SECTION("fails for an unrelated route") {
        REQUIRE_FALSE(ApiRouteParser::parseValveRoute("/api/status").has_value());
    }

    SECTION("fails for the bare prefix") {
        REQUIRE_FALSE(ApiRouteParser::parseValveRoute("/api/valve/").has_value());
    }

    SECTION("fails for an empty route") {
        REQUIRE_FALSE(ApiRouteParser::parseValveRoute("").has_value());
    }

    SECTION("fails for trailing garbage after the action") {
        REQUIRE_FALSE(ApiRouteParser::parseValveRoute("/api/valve/0/open/extra").has_value());
    }

    SECTION("fails for trailing garbage after the index") {
        REQUIRE_FALSE(ApiRouteParser::parseValveRoute("/api/valve/0abc/open").has_value());
    }
}
