#include "../include/ValveGroup.h"

ValveGroup::ValveGroup(
    std::array<Valve, 8> valves,
    const ITime &i_time
    ) noexcept:
    valves (std::move(valves)),
    i_time(i_time)
{ }

ValveGroup::ValveGroup(ValveGroup&& other) noexcept:
    isInitialized (other.isInitialized),
    valves (std::move(other.valves)),
    i_time (std::move(other.i_time)) {
    other.isInitialized = false;
}

ValveGroup::~ValveGroup() noexcept {
    if (isInitialized) {
        free();
    }
}

bool ValveGroup::initialize() noexcept {
    if (isInitialized) {
        return false;
    }

    for (auto& valve : valves) {
        if (valve.getPinNumber() != GPIO_NUM_NC) {
            if (!valve.initialize()) {
                return false;
            }
        }
    }

    isInitialized = true;

    return true;
}

bool ValveGroup::free() noexcept {
    if (!isInitialized) {
        return false;
    }

    for (auto& valve : valves) {
        if (valve.isReady()) {
            if (!valve.free()) {
                return false;
            }
        }
    }

    isInitialized = false;
    return true;
}

bool ValveGroup::isReady() const noexcept {
    return isInitialized;
}

bool ValveGroup::open(std::size_t index) noexcept {
    if (index >= valves.size()) {
        return false;
    }

    Valve& valve = valves[index];
    if (isInitialized || valve.isReady()) {
        return valve.open();
    }

    return false;
}

bool ValveGroup::close(std::size_t index) noexcept {
    if (index >= valves.size()) {
        return false;
    }

    Valve& valve = valves[index];
    if (isInitialized || valve.isReady()) {
        return valve.close();
    }

    return false;
}

bool ValveGroup::setOpenState(std::size_t index, bool openState) noexcept {
    if (index >= valves.size()) {
        return false;
    }

    Valve& valve = valves[index];
    if (isInitialized || valve.isReady()) {
        return valve.setOpenState(openState);
    }

    return false;
}

bool ValveGroup::autoCloseValvesAfterTimeoutPoll() noexcept {
    constexpr int NUM_SECONDS_TIMEOUT = 3600; // TODO: replace with nvs setting
    if (isInitialized) {
        for (auto& valve : valves) {
            if (
                valve.isReady() &&
                valve.getIsOpen() &&
                i_time.getSecondsSince(valve.getLastOpenedAtTime()) >= NUM_SECONDS_TIMEOUT
                ) {
                valve.close();
            }
        }
        return true;
    }
    return false;
}
