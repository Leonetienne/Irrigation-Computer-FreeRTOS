#include "../include/ValveGroup.h"

ValveGroup::ValveGroup(
    const ITime &i_time
    ) noexcept:
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

ValveGroup& ValveGroup::operator=(ValveGroup&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    if (isInitialized) {
        free();
    }

    // Reset (not just free) before taking over other's valves: if both optionals were
    // still engaged, optional::operator= would assign element-wise via Valve::operator=,
    // which intentionally leaves pinNum untouched - corrupting pin identity when the two
    // groups use different pins. Resetting forces re-construction instead.
    valves.reset();

    // i_time is left untouched (reference member, bound at construction)
    isInitialized = other.isInitialized;
    valves = std::move(other.valves);

    other.isInitialized = false;

    return *this;
}

bool ValveGroup::initialize(std::array<Valve, 8> newValves) noexcept {
    if (isInitialized) {
        return false;
    }

    valves.emplace(std::move(newValves));

    for (auto& valve : *valves) {
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

    for (auto& valve : *valves) {
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
    if (!isInitialized || index >= valves->size()) {
        return false;
    }

    Valve& valve = (*valves)[index];
    if (valve.isReady()) {
        return valve.open();
    }

    return false;
}

bool ValveGroup::close(std::size_t index) noexcept {
    if (!isInitialized || index >= valves->size()) {
        return false;
    }

    Valve& valve = (*valves)[index];
    if (valve.isReady()) {
        return valve.close();
    }

    return false;
}

bool ValveGroup::setOpenState(std::size_t index, bool openState) noexcept {
    if (!isInitialized || index >= valves->size()) {
        return false;
    }

    Valve& valve = (*valves)[index];
    if (valve.isReady()) {
        return valve.setOpenState(openState);
    }

    return false;
}

bool ValveGroup::autoCloseValvesAfterTimeoutPoll() noexcept {
    constexpr int NUM_SECONDS_TIMEOUT = 3600; // TODO: replace with nvs setting
    if (isInitialized) {
        for (auto& valve : *valves) {
            if (
                valve.isReady() &&
                valve.getIsOpen().value_or(false) &&
                i_time.getSecondsSince(valve.getLastOpenedAtTime()) >= NUM_SECONDS_TIMEOUT
                ) {
                if (!valve.close()) {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

std::expected<bool, bool> ValveGroup::getValveOpenState(std::size_t index) const noexcept {
    if (!isInitialized || index >= valves->size()) {
        return std::unexpected(false);
    }

    const Valve& valve = (*valves)[index];
    if (valve.isReady()) {
        return valve.getIsOpen();
    }

    return std::unexpected(false);
}
