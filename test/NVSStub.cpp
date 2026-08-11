#include <catch2/catch_test_macros.hpp>
#include "test/stubs/NVSStub.h"

TEST_CASE("NVSStub", "[NVSStub]") {
    NVSStub stub;

    SECTION("setInt fails before begin") {
        REQUIRE_FALSE(stub.setInt("count", 42));
    }

    SECTION("getInt fails before begin") {
        int32_t value = 0;
        REQUIRE_FALSE(stub.getInt("count", value));
    }

    SECTION("begin records the namespace") {
        stub.begin("irrigation");
        REQUIRE(stub.getLastNamespace() == "irrigation");
    }

    SECTION("begin increments the call count") {
        stub.begin("irrigation");
        REQUIRE(stub.getBeginCallCount() == 1);
    }

    SECTION("begin fails when already initialized") {
        REQUIRE(stub.begin("irrigation"));
        REQUIRE_FALSE(stub.begin("irrigation"));
    }

    SECTION("free fails before begin") {
        REQUIRE_FALSE(stub.free());
    }

    SECTION("free succeeds after begin") {
        stub.begin("irrigation");
        REQUIRE(stub.free());
    }

    SECTION("free fails when called twice") {
        stub.begin("irrigation");
        REQUIRE(stub.free());
        REQUIRE_FALSE(stub.free());
    }

    SECTION("setInt/getInt round trip") {
        stub.begin("irrigation");
        REQUIRE(stub.setInt("count", 42));

        int32_t value = 0;
        REQUIRE(stub.getInt("count", value));
        REQUIRE(value == 42);
    }

    SECTION("getInt fails for an unknown key") {
        stub.begin("irrigation");

        int32_t value = 0;
        REQUIRE_FALSE(stub.getInt("missing", value));
    }

    SECTION("setInt overwrites a previously stored value") {
        stub.begin("irrigation");
        stub.setInt("count", 1);
        stub.setInt("count", 2);

        int32_t value = 0;
        REQUIRE(stub.getInt("count", value));
        REQUIRE(value == 2);
    }

    SECTION("setString fails before begin") {
        REQUIRE_FALSE(stub.setString("ssid", "my_example_ap"));
    }

    SECTION("getString fails before begin") {
        char buffer[NVS_MAX_STRING_LENGTH + 1];
        REQUIRE_FALSE(stub.getString("ssid", buffer));
    }

    SECTION("setString/getString round trip") {
        stub.begin("irrigation");
        REQUIRE(stub.setString("ssid", "my_example_ap"));

        char buffer[NVS_MAX_STRING_LENGTH + 1];
        REQUIRE(stub.getString("ssid", buffer));
        REQUIRE(std::string(buffer) == "my_example_ap");
    }

    SECTION("getString fails for an unknown key") {
        stub.begin("irrigation");

        char buffer[NVS_MAX_STRING_LENGTH + 1];
        REQUIRE_FALSE(stub.getString("missing", buffer));
    }

    SECTION("setString fails for a value longer than NVS_MAX_STRING_LENGTH") {
        stub.begin("irrigation");
        const std::string tooLong(NVS_MAX_STRING_LENGTH + 1, 'a');
        REQUIRE_FALSE(stub.setString("ssid", tooLong.c_str()));
    }

    SECTION("setString accepts a value exactly NVS_MAX_STRING_LENGTH characters long") {
        stub.begin("irrigation");
        const std::string maxLength(NVS_MAX_STRING_LENGTH, 'a');
        REQUIRE(stub.setString("ssid", maxLength.c_str()));
    }

    SECTION("eraseKey fails before begin") {
        REQUIRE_FALSE(stub.eraseKey("ssid"));
    }

    SECTION("eraseKey removes a previously stored string value") {
        stub.begin("irrigation");
        stub.setString("ssid", "my_example_ap");

        REQUIRE(stub.eraseKey("ssid"));

        char buffer[NVS_MAX_STRING_LENGTH + 1];
        REQUIRE_FALSE(stub.getString("ssid", buffer));
    }

    SECTION("eraseKey removes a previously stored int value") {
        stub.begin("irrigation");
        stub.setInt("count", 42);

        REQUIRE(stub.eraseKey("count"));

        int32_t value = 0;
        REQUIRE_FALSE(stub.getInt("count", value));
    }

    SECTION("eraseKey succeeds for a key that was never set") {
        stub.begin("irrigation");
        REQUIRE(stub.eraseKey("missing"));
    }
}
