#include "../include/Valve.h"

Valve::Valve(gpio_num_t gpioPinNumber, IGpio& gpio, GpioPinRegister& pinRegister) noexcept:
    gpioPin ( pinRegister, gpio, gpioPinNumber)
{ }

Valve::Valve(Valve &&other) noexcept :
    isOpen (other.isOpen),
    isInitialized (other.isInitialized),
    gpioPin (std::move(other.gpioPin))
{
    other.isInitialized = false;
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

bool Valve::getIsOpen() const noexcept {
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

    isOpen = true;
    return gpioPin.setState(PIN_STATE_DIGITAL::HIGH); // Valve is OPEN when pin is HIGH (transistor opens)
}

bool Valve::close() noexcept {
    if (!isInitialized) {
        return false;
    }

    isOpen = false;
    return gpioPin.setState(PIN_STATE_DIGITAL::LOW); // Valve is CLOSED when pin is LOW (transistor closes)
}
