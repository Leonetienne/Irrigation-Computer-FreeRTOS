#include <catch2/catch_test_macros.hpp>
#include "../main/include/GpioPinRegister.h"
#include "test/stubs/GpioStub.h"
#include "test/stubs/TimeStub.h"
#include "../main/include/Valve.h"
#include "../main/include/ValveGroup.h"
#include "../main/include/ApiController.h"

TEST_CASE("ApiController: executeValveOperation", "[ApiController]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};

    std::array<Valve, 8> valves = {
        Valve(GPIO_NUM_0, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_1, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_2, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_3, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_4, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_5, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_6, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_7, gpioStub, timeStub, pr),
    };

    ValveGroup group(timeStub);
    REQUIRE(group.initialize(std::move(valves)));

    SECTION("Open opens the target valve") {
        REQUIRE(ApiController::executeValveOperation(group, ValveCommand{3, ValveAction::Open}));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("Close closes the target valve") {
        REQUIRE(ApiController::executeValveOperation(group, ValveCommand{3, ValveAction::Open}));
        REQUIRE(ApiController::executeValveOperation(group, ValveCommand{3, ValveAction::Close}));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("Open only affects the target valve") {
        REQUIRE(ApiController::executeValveOperation(group, ValveCommand{2, ValveAction::Open}));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_2) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_1) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("fails for an out-of-bounds index") {
        REQUIRE_FALSE(ApiController::executeValveOperation(group, ValveCommand{8, ValveAction::Open}));
    }
}

TEST_CASE("ApiController: executeValveOperation before group init", "[ApiController]") {
    TimeStub timeStub{};
    ValveGroup group(timeStub);

    SECTION("Open fails") {
        REQUIRE_FALSE(ApiController::executeValveOperation(group, ValveCommand{0, ValveAction::Open}));
    }

    SECTION("Close fails") {
        REQUIRE_FALSE(ApiController::executeValveOperation(group, ValveCommand{0, ValveAction::Close}));
    }
}
