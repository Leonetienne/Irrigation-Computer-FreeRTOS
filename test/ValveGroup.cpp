//
// Created by Agent on 08.08.26.
//

#include <catch2/catch_test_macros.hpp>
#include "../main/include/GpioPinRegister.h"
#include "test/stubs/GpioStub.h"
#include "test/stubs/TimeStub.h"
#include "test/stubs/NVSStub.h"
#include "../main/include/Valve.h"
#include "../main/include/ValveGroup.h"
#include "../main/include/SettingsManager.h"

TEST_CASE("ValveGroup: lifecycle", "[ValveGroup]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

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

    ValveGroup group(timeStub, settings);

    SECTION("not initialized by default") {
        REQUIRE_FALSE(group.isReady());
    }

    SECTION("initialize succeeds with all valid pins") {
        REQUIRE(group.initialize(std::move(valves)));
    }

    SECTION("isReady returns true after initialize") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.isReady());
    }

    SECTION("initialize twice fails") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE_FALSE(group.initialize(std::move(valves)));
    }

    SECTION("open sets gpio to HIGH after init") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.open(3));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("close sets gpio to LOW after init") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.open(3));
        REQUIRE(group.close(3));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("setOpenState(true) opens the valve") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.setOpenState(5, true));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_5) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("setOpenState(false) closes the valve") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.open(5));
        REQUIRE(group.setOpenState(5, false));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_5) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("open fails before init") {
        REQUIRE_FALSE(group.open(0));
    }

    SECTION("close fails before init") {
        REQUIRE_FALSE(group.close(0));
    }

    SECTION("setOpenState fails before init") {
        REQUIRE_FALSE(group.setOpenState(0, true));
    }

    SECTION("open fails with out-of-bounds index") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE_FALSE(group.open(8));
        REQUIRE_FALSE(group.open(-1));
    }

    SECTION("close fails with out-of-bounds index") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE_FALSE(group.close(8));
        REQUIRE_FALSE(group.close(-1));
    }

    SECTION("setOpenState fails with out-of-bounds index") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE_FALSE(group.setOpenState(8, true));
        REQUIRE_FALSE(group.setOpenState(-1, true));
    }

    SECTION("free releases all initialized valves") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(pr.isPinBound(GPIO_NUM_0));
        REQUIRE(pr.isPinBound(GPIO_NUM_7));
        REQUIRE(group.free());
        REQUIRE_FALSE(pr.isPinBound(GPIO_NUM_0));
        REQUIRE_FALSE(pr.isPinBound(GPIO_NUM_7));
    }

    SECTION("free fails before init") {
        REQUIRE_FALSE(group.free());
    }

    SECTION("free fails after free") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.free());
        REQUIRE_FALSE(group.free());
    }

    SECTION("isReady returns false after free") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.free());
        REQUIRE_FALSE(group.isReady());
    }

    SECTION("open fails after free") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.free());
        REQUIRE_FALSE(group.open(0));
    }

    SECTION("close fails after free") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.free());
        REQUIRE_FALSE(group.close(0));
    }

    SECTION("setOpenState fails after free") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.free());
        REQUIRE_FALSE(group.setOpenState(0, true));
    }

    SECTION("dtor after free() does not double-free") {
        {
            std::array<Valve, 8> scopedValves = {
                Valve(GPIO_NUM_8, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_9, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_10, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_11, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_12, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_13, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_14, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_15, gpioStub, timeStub, pr),
            };
            ValveGroup scopedGroup(timeStub, settings);
            REQUIRE(scopedGroup.initialize(std::move(scopedValves)));
            REQUIRE(scopedGroup.free());
        }
        SUCCEED("dtor after explicit free() did not double-free");
    }

    SECTION("dtor calls free when not freed before") {
        {
            std::array<Valve, 8> scopedValves = {
                Valve(GPIO_NUM_16, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_17, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_18, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_19, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_20, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_21, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
                Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
            };
            ValveGroup scopedGroup(timeStub, settings);
            REQUIRE(scopedGroup.initialize(std::move(scopedValves)));
            REQUIRE(pr.isPinBound(GPIO_NUM_16));
        }
        REQUIRE_FALSE(pr.isPinBound(GPIO_NUM_16));
    }
}

TEST_CASE("ValveGroup: each valve index maps to correct gpio pin", "[ValveGroup]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

    std::array<Valve, 8> valves = {
        Valve(GPIO_NUM_10, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_11, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_12, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_13, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_14, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_15, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_16, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_17, gpioStub, timeStub, pr),
    };

    ValveGroup group(timeStub, settings);
    REQUIRE(group.initialize(std::move(valves)));

    SECTION("open valve 0 only sets GPIO_NUM_10 HIGH") {
        REQUIRE(group.open(0));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_10) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_11) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_12) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_13) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_14) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_15) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_16) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_17) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("open valve 3 only sets GPIO_NUM_13 HIGH") {
        REQUIRE(group.open(3));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_10) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_11) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_12) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_13) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_14) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_15) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_16) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_17) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("open valve 7 only sets GPIO_NUM_17 HIGH") {
        REQUIRE(group.open(7));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_10) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_11) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_12) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_13) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_14) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_15) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_16) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_17) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("opening different valves sets only the correct pins") {
        REQUIRE(group.open(1));
        REQUIRE(group.open(5));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_10) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_11) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_12) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_13) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_14) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_15) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_16) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_17) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }
}

TEST_CASE("ValveGroup: initialize skips NC pins", "[ValveGroup]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

    std::array<Valve, 8> valves = {
        Valve(GPIO_NUM_0, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_2, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_4, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_6, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
    };

    ValveGroup group(timeStub, settings);

    SECTION("initialize succeeds with mixed valid and NC pins") {
        REQUIRE(group.initialize(std::move(valves)));
    }

    SECTION("NC-pin valves cannot open after group init") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE_FALSE(group.open(1));
        REQUIRE_FALSE(group.open(3));
        REQUIRE_FALSE(group.open(5));
        REQUIRE_FALSE(group.open(7));
    }

    SECTION("valid-pin valves can open after group init") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.open(0));
        REQUIRE(group.open(2));
        REQUIRE(group.open(4));
        REQUIRE(group.open(6));
    }
}

TEST_CASE("ValveGroup: move", "[ValveGroup]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

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

    ValveGroup group(timeStub, settings);

    SECTION("moved-from group reports not ready") {
        REQUIRE(group.initialize(std::move(valves)));
        ValveGroup moved{std::move(group)};
        REQUIRE_FALSE(group.isReady());
    }

    SECTION("moved-to group can operate valves") {
        REQUIRE(group.initialize(std::move(valves)));
        ValveGroup moved{std::move(group)};
        REQUIRE(moved.open(0));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_0) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("moved-from group cannot open valves") {
        REQUIRE(group.initialize(std::move(valves)));
        ValveGroup moved{std::move(group)};
        REQUIRE_FALSE(group.open(0));
    }

    SECTION("moved-to group can free") {
        REQUIRE(group.initialize(std::move(valves)));
        ValveGroup moved{std::move(group)};
        REQUIRE(pr.isPinBound(GPIO_NUM_0));
        REQUIRE(moved.free());
        REQUIRE_FALSE(pr.isPinBound(GPIO_NUM_0));
    }

    SECTION("move-assigning into an already-initialized group frees its own valves first") {
        REQUIRE(group.initialize(std::move(valves)));

        std::array<Valve, 8> otherValves = {
            Valve(GPIO_NUM_20, gpioStub, timeStub, pr),
            Valve(GPIO_NUM_21, gpioStub, timeStub, pr),
            Valve(GPIO_NUM_22, gpioStub, timeStub, pr),
            Valve(GPIO_NUM_23, gpioStub, timeStub, pr),
            Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
            Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
            Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
            Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
        };
        ValveGroup other(timeStub, settings);
        REQUIRE(other.initialize(std::move(otherValves)));
        REQUIRE(pr.isPinBound(GPIO_NUM_0));
        REQUIRE(pr.isPinBound(GPIO_NUM_20));

        group = std::move(other);

        REQUIRE_FALSE(pr.isPinBound(GPIO_NUM_0));
        REQUIRE(pr.isPinBound(GPIO_NUM_20));
        REQUIRE(group.open(0));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_20) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }
}

TEST_CASE("ValveGroup: auto close timeout", "[ValveGroup]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

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

    ValveGroup group(timeStub, settings);

    SECTION("closes valves open longer than the configured timeout") {
        REQUIRE(settings.storeRuntimeSafetyEnabled(true));
        REQUIRE(settings.storeMaxValveRuntime(60)); // minutes -> 3600s

        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.open(0));
        REQUIRE(group.open(3));

        timeStub.setStubbedTime(timeStub.getTime() + 3601);

        REQUIRE(group.autoCloseValvesAfterTimeoutPoll());

        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_0) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_3) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("does not close valves under the configured timeout") {
        REQUIRE(settings.storeRuntimeSafetyEnabled(true));
        REQUIRE(settings.storeMaxValveRuntime(60)); // minutes -> 3600s

        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.open(0));

        timeStub.setStubbedTime(timeStub.getTime() + 1800);

        REQUIRE(group.autoCloseValvesAfterTimeoutPoll());

        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_0) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("respects a shorter configured timeout") {
        REQUIRE(settings.storeRuntimeSafetyEnabled(true));
        REQUIRE(settings.storeMaxValveRuntime(1)); // minutes -> 60s

        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.open(0));

        timeStub.setStubbedTime(timeStub.getTime() + 59);
        REQUIRE(group.autoCloseValvesAfterTimeoutPoll());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_0) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));

        timeStub.setStubbedTime(timeStub.getTime() + 2);
        REQUIRE(group.autoCloseValvesAfterTimeoutPoll());
        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_0) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("does not close valves when runtime safety is disabled") {
        REQUIRE(settings.storeRuntimeSafetyEnabled(false));
        REQUIRE(settings.storeMaxValveRuntime(1)); // minutes -> 60s, would otherwise trigger

        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.open(0));

        timeStub.setStubbedTime(timeStub.getTime() + 3600);

        group.autoCloseValvesAfterTimeoutPoll();

        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_0) == static_cast<uint32_t>(PIN_STATE_DIGITAL::HIGH));
    }

    SECTION("defaults to enabled when nothing was stored") {
        REQUIRE(settings.storeMaxValveRuntime(1)); // minutes -> 60s

        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.open(0));

        timeStub.setStubbedTime(timeStub.getTime() + 61);
        REQUIRE(group.autoCloseValvesAfterTimeoutPoll());

        REQUIRE(gpioStub.test_gpioGetLevel(GPIO_NUM_0) == static_cast<uint32_t>(PIN_STATE_DIGITAL::LOW));
    }

    SECTION("returns false before init") {
        REQUIRE_FALSE(group.autoCloseValvesAfterTimeoutPoll());
    }
}

TEST_CASE("ValveGroup: getValveOpenState", "[ValveGroup]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

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

    ValveGroup group(timeStub, settings);

    SECTION("returns error before init") {
        REQUIRE_FALSE(group.getValveOpenState(0).has_value());
    }

    SECTION("returns error for out-of-bounds index") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE_FALSE(group.getValveOpenState(8).has_value());
    }

    SECTION("returns false for a closed valve after init") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.getValveOpenState(0) == false);
    }

    SECTION("returns true for an open valve after init") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.open(3));
        REQUIRE(group.getValveOpenState(3) == true);
    }

    SECTION("returns error after free") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.free());
        REQUIRE_FALSE(group.getValveOpenState(0).has_value());
    }
}

TEST_CASE("ValveGroup: isValveOperable", "[ValveGroup]") {
    GpioPinRegister pr{};
    GpioStub gpioStub{};
    TimeStub timeStub{};
    NVSStub nvs{};
    REQUIRE(nvs.begin("system"));
    SettingsManager settings(nvs);

    std::array<Valve, 8> valves = {
        Valve(GPIO_NUM_0, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_2, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_4, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_6, gpioStub, timeStub, pr),
        Valve(GPIO_NUM_NC, gpioStub, timeStub, pr),
    };

    ValveGroup group(timeStub, settings);

    SECTION("returns false before init") {
        REQUIRE_FALSE(group.isValveOperable(0));
    }

    SECTION("returns true for a valve with a configured pin") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.isValveOperable(0));
    }

    SECTION("returns false for a valve with no pin configured") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE_FALSE(group.isValveOperable(1));
    }

    SECTION("returns false for an out-of-bounds index") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE_FALSE(group.isValveOperable(8));
    }

    SECTION("returns false after free") {
        REQUIRE(group.initialize(std::move(valves)));
        REQUIRE(group.free());
        REQUIRE_FALSE(group.isValveOperable(0));
    }
}
