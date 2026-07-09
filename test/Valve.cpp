//
// Created by Leon Etienne on 21.03.26.
//

#include <catch2/catch_test_macros.hpp>
#include "../main/include/GpioPinRegister.h"
#include "test/stubs/GpioStub.h"
#include "test/stubs/TimeStub.h"
#include "../main/include/Valve.h"

#include "test/stubs/TimeStub.h"

TEST_CASE("Valve: lifecycle", "[Valve]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    Valve valve(
        GPIO_NUM_19,
        gpioStub,
        timeStub,
        pr
    );

    SECTION("not open by default") {
        REQUIRE_FALSE(valve.getIsOpen());
    }

    SECTION("not initialized by default") {
        REQUIRE_FALSE(valve.isReady());
    }

    SECTION("can initialize") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.isReady());
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

    SECTION("last_opened_at is 0 before initialization") {
        REQUIRE(valve.getLastOpenedAtTime() == 0);
    }

    SECTION("last_opened_at is 0 after initialization") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.getLastOpenedAtTime() == 0);
    }

    SECTION("last_opened_at is set after opening") {
        timeStub.setStubbedTime(1500000000);
        REQUIRE(valve.initialize());
        REQUIRE(valve.open());
        REQUIRE(valve.getLastOpenedAtTime() == 1500000000);
    }

    SECTION("last_opened_at is not unset after closing") {
        timeStub.setStubbedTime(1500000000);
        REQUIRE(valve.initialize());
        REQUIRE(valve.open());
        REQUIRE(valve.close());
        REQUIRE(valve.getLastOpenedAtTime() == 1500000000);
    }

    SECTION("last_opened_at gets overridden by next open") {
        timeStub.setStubbedTime(1500000000);
        REQUIRE(valve.initialize());
        REQUIRE(valve.open());
        REQUIRE(valve.close());
        timeStub.setStubbedTime(1500001000);
        REQUIRE(valve.open());
        REQUIRE(valve.getLastOpenedAtTime() == 1500001000);
    }

    SECTION("last_opened_at is not overridden by successive open") {
        timeStub.setStubbedTime(1500000000);
        REQUIRE(valve.initialize());
        REQUIRE(valve.open());
        timeStub.setStubbedTime(1500001000);
        REQUIRE(valve.open());
        REQUIRE(valve.getLastOpenedAtTime() == 1500000000);
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

    SECTION("free succeeds after init") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.free());
    }

    SECTION("free fails before init") {
        REQUIRE_FALSE(valve.free());
    }

    SECTION("free clears ready state") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.free());
        REQUIRE_FALSE(valve.isReady());
    }

    SECTION("free unregisters the pin") {
        REQUIRE(valve.initialize());
        REQUIRE(pr.isPinBound(GPIO_NUM_19));
        REQUIRE(valve.free());
        REQUIRE_FALSE(pr.isPinBound(GPIO_NUM_19));
    }

    SECTION("free();free() fails, can't free twice") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.free());
        REQUIRE_FALSE(valve.free());
    }

    SECTION("open fails after free") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.free());
        REQUIRE_FALSE(valve.open());
    }

    SECTION("close fails after free") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.free());
        REQUIRE_FALSE(valve.close());
    }

    SECTION("setOpenState fails after free") {
        REQUIRE(valve.initialize());
        REQUIRE(valve.free());
        REQUIRE_FALSE(valve.setOpenState(true));
    }

    SECTION("dtor calls free only when free was not called before") {
        // free() called explicitly, dtor must not double-free
        {
            Valve v(
                GPIO_NUM_19,
                gpioStub,
                timeStub,
                pr
            );
            REQUIRE(v.initialize());
            REQUIRE(v.free());
        }
        // No crash = dtor didn't double-free
        SUCCEED("dtor after explicit free() did not double-free");
    }

    SECTION("dtor calls free when free was not called before") {
        // free() NOT called, dtor must call it
        {
             Valve v(
                GPIO_NUM_19,
                gpioStub,
                timeStub,
                pr
            );
            REQUIRE(v.initialize());
            REQUIRE(pr.isPinBound(GPIO_NUM_19));
        }
        // After dtor ran, the pin should be unregistered
        REQUIRE_FALSE(pr.isPinBound(GPIO_NUM_19));
    }
}

TEST_CASE("Valve: move", "[Valve]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    Valve valve(
        GPIO_NUM_19,
        gpioStub,
        timeStub,
        pr
    );

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
