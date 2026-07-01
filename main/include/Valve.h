#include "platform/GpioDigitalWritePin.h"
#include "hal/IGpio.h"
#include "GpioPinRegister.h"
#include "compat/gpio_num_t.h"

#ifndef IRRIGATION_COMPUTER_TESTS_VALVE_H
#define IRRIGATION_COMPUTER_TESTS_VALVE_H

/**
 * A watering channel that can either be closed or open
 */
class Valve {
public:
    Valve(gpio_num_t gpioPinNumber, IGpio& gpio, GpioPinRegister& pinRegister) noexcept;
    Valve(const Valve&) = delete;
    Valve(Valve&& other) noexcept;
    ~Valve() noexcept;

    /**
     * Will return whether this valve is fully initialized
     * @return Success state
     */
    [[nodiscard]] bool isReady() const noexcept;

    /**
     * Will initialize the valve
     * @return Success state
     */
    bool initialize() noexcept;

    /**
     * Will free the resources owned by this valve
     * @return Success state
     */
    bool free() noexcept;

    /**
     * Copy-assignment operator
     */
    void operator=(const Valve&) = delete;

    /**
     * @return Whether the valve is currently open
     */
    [[nodiscard]] bool getIsOpen() const noexcept;

    /**
     * Opens or closes the valve.
     * QoL-mapping for open/close
     * @param openState true if it should be open
     * @return Success state
     */
    bool setOpenState(bool openState) noexcept;

    /**
     * opens the valve
     * @return Success state
     */
    bool open() noexcept;

    /**
     * Closes the valve
     * @return Success state
     */
    bool close() noexcept;

private:
    bool isOpen = false;
    bool isInitialized = false;
    GpioDigitalWritePin gpioPin;
};


#endif //IRRIGATION_COMPUTER_TESTS_VALVE_H
