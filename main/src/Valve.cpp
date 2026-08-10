#include "../include/Valve.h"

Valve::Valve(
    gpio_num_t actuatorGpioPinNumber,
    gpio_num_t indicatorGpioPinNumber,
    IGpio& i_gpio,
    const ITime& i_time,
    GpioPinRegister& pinRegister
) noexcept:
    actuatorGpioPin ( pinRegister, i_gpio, actuatorGpioPinNumber),
    indicatorGpioPin ( pinRegister, i_gpio, indicatorGpioPinNumber),
    i_time (i_time)
{ }

Valve::Valve(Valve &&other) noexcept :
    isOpen (other.isOpen),
    isInitialized (other.isInitialized),
    actuatorGpioPin (std::move(other.actuatorGpioPin)),
    indicatorGpioPin (std::move(other.indicatorGpioPin)),
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

    if (!actuatorGpioPin.initialize()) {
        return false;
    }

    // No indicator gpio is no fail as these are optional
    if (indicatorGpioPin.getGpioNum() != GPIO_NUM_NC) {
        if (!indicatorGpioPin.initialize()) {
            return false;
        }
    }

    isInitialized = true;
    return true;
}

bool Valve::free() noexcept {
    if (isInitialized) {
        // We set this to uninitialized right away to prevent an aborted free-call resulting in isInitialized still being true
        isInitialized = false;

        if (!actuatorGpioPin.free()) {
            return false;
        }

        // Indicator pins are optional
        if (indicatorGpioPin.isReady() && !indicatorGpioPin.free()) {
            return false;
        }

        return true;
    }
    return false;
}

Valve& Valve::operator=(Valve&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    if (isInitialized) {
        free();
    }

    // i_time is left untouched (reference member, bound at construction)
    isOpen = other.isOpen;
    isInitialized = other.isInitialized;
    actuatorGpioPin = std::move(other.actuatorGpioPin);
    indicatorGpioPin = std::move(other.indicatorGpioPin);
    lastOpenedAt = other.lastOpenedAt;

    other.isInitialized = false;

    return *this;
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

    // Turn status LED on
    if (indicatorGpioPin.isReady()) {
        indicatorGpioPin.setState(PIN_STATE_DIGITAL::HIGH);
    }

    lastOpenedAt = i_time.getTime();
    return actuatorGpioPin.setState(PIN_STATE_DIGITAL::HIGH); // Valve is OPEN when pin is HIGH (transistor opens)
}

bool Valve::close() noexcept {
    if (!isInitialized) {
        return false;
    }

    // Fast-accept
    if (!isOpen) {
        return true;
    }

    // Turn status LED off
    if (indicatorGpioPin.isReady()) {
        indicatorGpioPin.setState(PIN_STATE_DIGITAL::LOW);
    }

    isOpen = false;
    return actuatorGpioPin.setState(PIN_STATE_DIGITAL::LOW); // Valve is CLOSED when pin is LOW (transistor closes)
}

time_t Valve::getLastOpenedAtTime() const noexcept {
    return lastOpenedAt;
}

gpio_num_t Valve::getPinNumber() const noexcept {
    return actuatorGpioPin.getGpioNum();
}
