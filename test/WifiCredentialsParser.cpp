#include <catch2/catch_test_macros.hpp>
#include "WifiCredentialsParser.h"

TEST_CASE("WifiCredentialsParser: parse", "[WifiCredentialsParser]") {
    SECTION("parses ssid and password") {
        const auto credentials = WifiCredentialsParser::parse("ssid=MyHomeWifi&password=hunter2");
        REQUIRE(credentials.has_value());
        REQUIRE(credentials->ssid == "MyHomeWifi");
        REQUIRE(credentials->password == "hunter2");
    }

    SECTION("order of fields does not matter") {
        const auto credentials = WifiCredentialsParser::parse("password=hunter2&ssid=MyHomeWifi");
        REQUIRE(credentials.has_value());
        REQUIRE(credentials->ssid == "MyHomeWifi");
        REQUIRE(credentials->password == "hunter2");
    }

    SECTION("allows an empty password (open networks)") {
        const auto credentials = WifiCredentialsParser::parse("ssid=MyHomeWifi&password=");
        REQUIRE(credentials.has_value());
        REQUIRE(credentials->password.empty());
    }

    SECTION("decodes percent-encoded characters") {
        const auto credentials = WifiCredentialsParser::parse("ssid=My%20Wifi&password=p%40ss%21");
        REQUIRE(credentials.has_value());
        REQUIRE(credentials->ssid == "My Wifi");
        REQUIRE(credentials->password == "p@ss!");
    }

    SECTION("decodes plus signs as spaces") {
        const auto credentials = WifiCredentialsParser::parse("ssid=My+Home+Wifi&password=hunter2");
        REQUIRE(credentials.has_value());
        REQUIRE(credentials->ssid == "My Home Wifi");
    }

    SECTION("ignores unrelated fields") {
        const auto credentials = WifiCredentialsParser::parse("foo=bar&ssid=MyHomeWifi&password=hunter2&baz=qux");
        REQUIRE(credentials.has_value());
        REQUIRE(credentials->ssid == "MyHomeWifi");
        REQUIRE(credentials->password == "hunter2");
    }

    SECTION("fails when ssid is missing") {
        REQUIRE_FALSE(WifiCredentialsParser::parse("password=hunter2").has_value());
    }

    SECTION("fails when ssid is empty") {
        REQUIRE_FALSE(WifiCredentialsParser::parse("ssid=&password=hunter2").has_value());
    }

    SECTION("fails when password is missing") {
        REQUIRE_FALSE(WifiCredentialsParser::parse("ssid=MyHomeWifi").has_value());
    }

    SECTION("fails for an empty body") {
        REQUIRE_FALSE(WifiCredentialsParser::parse("").has_value());
    }

    SECTION("handles a trailing invalid percent escape as literal characters") {
        const auto credentials = WifiCredentialsParser::parse("ssid=abc%&password=hunter2");
        REQUIRE(credentials.has_value());
        REQUIRE(credentials->ssid == "abc%");
    }
}
