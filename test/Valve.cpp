//
// Created by Leon Etienne on 21.03.26.
//

#include <catch2/catch_test_macros.hpp>
#include "../main/include/GpioPinRegister.h"
#include "test/stubs/GpioStub.h"
#include "../main/include/Valve.h"

TEST_CASE("Valve: lifecycle", "[Valve]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    Valve valve{GPIO_NUM_19, gpioStub, pr};

    SECTION("not open by default") {
        REQUIRE_FALSE(valve.getIsOpen());
    }

    SECTION("can initialize") {
        REQUIRE(valve.initialize());
    }

    SECTION("can't initialize twice") {
        REQUIRE(valve.initialize());
        REQUIRE_FALSE(valve.initialize());
    }

    SECTION("gpio pin direction set to output after init") {
        REQUIRE(valve.initialize());
        REQUIRE(gpioStub.test_gpioGetMode(GPIO_NUM_19) == GPIO_MODE_OUTPUT);
    }

    SECTION("pin registered after init") {
        REQUIRE(valve.initialize());
        REQUIRE(pr.isPinBound(GPIO_NUM_19));
    }

    SECTION("open fails before init") {
        REQUIRE_FALSE(valve.open());
    }

    SECTION("close fails before init") {
        REQUIRE_FALSE(valve.close());
    }

    SECTION("setOpenState fails before init") {
        REQUIRE_FALSE(valve.setOpenState(true));
    }

    SECTION("can open after init") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.open());
    }

    SECTION("isOpen reports true after open") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.open());
        REQUIRE(valve.getIsOpen());
    }

    SECTION("open sets gpio to HIGH") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.open());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_19) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("can close after init") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.close());
    }

    SECTION("isOpen stays false after close") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.close());
        REQUIRE_FALSE(valve.getIsOpen());
    }

    SECTION("close sets gpio to LOW") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.close());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_19) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("can switch from open to close") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.open());
        REQUIRE(valve.getIsOpen());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_19) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));

        REQUIRE(valve.close());
        REQUIRE_FALSE(valve.getIsOpen());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_19) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("can switch from close to open") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.close());
        REQUIRE_FALSE(valve.getIsOpen());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_19) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));

        REQUIRE(valve.open());
        REQUIRE(valve.getIsOpen());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_19) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("setOpenState(true) opens the valve") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.setOpenState(true));
        REQUIRE(valve.getIsOpen());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_19) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("setOpenState(false) closes the valve") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.open());
        REQUIRE(valve.setOpenState(false));
        REQUIRE_FALSE(valve.getIsOpen());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_19) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }
}

TEST_CASE("Valve: move", "[Valve]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    Valve valve{GPIO_NUM_19, gpioStub, pr};

    SECTION("moved valve retains initialized state") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.getIsOpen() == false);

        Valve moved{std::move(valve)};

        // Moved-from valve is quietly "not initialized" (isOpen == false)
        REQUIRE_FALSE(valve.getIsOpen());
        // Moved-to valve kept its state
        REQUIRE_FALSE(moved.getIsOpen());

        // New owner can operate normally
        REQUIRE(moved.open());
        REQUIRE(moved.getIsOpen());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_19) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("moved valve retains open state") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.open());

        Valve moved{std::move(valve)};

        REQUIRE(moved.getIsOpen());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_19) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("moved valve retains closed state") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.close());

        Valve moved{std::move(valve)};

        REQUIRE_FALSE(moved.getIsOpen());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_19) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }
}
