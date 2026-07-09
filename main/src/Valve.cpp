#include "../include/Valve.h"

Valve::Valve(
    gpio_num_t gpioPinNumber,
    IGpio& i_gpio,
    const ITime& i_time,
    GpioPinRegister& pinRegister
) noexcept:
    gpioPin ( pinRegister, i_gpio, gpioPinNumber),
    i_time (i_time)
{ }

Valve::Valve(Valve &&other) noexcept :
    isOpen (other.isOpen),
    isInitialized (other.isInitialized),
    gpioPin (std::move(other.gpioPin)),
    i_time (std::move(other.i_time))
{
    other.isInitialized = false;
}

Valve::~Valve() noexcept {
    if (isInitialized) {
        free();
    }
}

bool Valve::isReady() const noexcept {
    return isInitialized;
}

bool Valve::initialize() noexcept {
    if (isInitialized) {
        return false;
    }

    if (!gpioPin.initialize()) {
        return false;
    }

    isInitialized = true;
    return true;
}

bool Valve::free() noexcept {
    if (isInitialized) {
        // We set this to uninitialized right away to prevent an aborted free-call resulting in isInitialized still being true
        isInitialized = false;

        if (!gpioPin.free()) {
            return false;
        }

        return true;
    }
    return false;
}

std::expected<bool, bool> Valve::getIsOpen() const noexcept {
    if (!isInitialized) {
        return std::unexpected(false);
    }
    return isOpen;
}

bool Valve::setOpenState(bool openState) noexcept {
    if (!isInitialized) {
        return false;
    }

    if (openState) {
        return open();
    }
    return close();
}

bool Valve::open() noexcept {
    if (!isInitialized) {
        return false;
    }

    // Fast-accept (no need to set lastOpened at, as it already is open)
    if (isOpen) {
        return true;
    }

    isOpen = true;
    lastOpenedAt = i_time.getTime();
    return gpioPin.setState(PIN_STATE_DIGITAL::HIGH); // Valve is OPEN when pin is HIGH (transistor opens)
}

bool Valve::close() noexcept {
    if (!isInitialized) {
        return false;
    }

    // Fast-accept
    if (!isOpen) {
        return true;
    }


    isOpen = false;
    return gpioPin.setState(PIN_STATE_DIGITAL::LOW); // Valve is CLOSED when pin is LOW (transistor closes)
}

time_t Valve::getLastOpenedAtTime() const noexcept {
    return lastOpenedAt;
}

gpio_num_t Valve::getPinNumber() const noexcept {
    return gpioPin.getGpioNum();
}
