#include "platform/GpioDigitalWritePin.h"
#include "hal/IGpio.h"
#include "hal/ITime.h"
#include "GpioPinRegister.h"

#ifndef IRRIGATION_COMPUTER_TESTS_VALVE_H
#define IRRIGATION_COMPUTER_TESTS_VALVE_H

/**
 * A watering channel that can either be closed or open
 */
class Valve {
public:
    Valve(
        gpio_num_t gpioPinNumber,
        IGpio& gpio,
        ITime& time,
        GpioPinRegister& pinRegister
    ) noexcept;
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

    /**
     * @return Time time_t this valve was last opened at
     */
    [[nodiscard]] time_t getLastOpenedAtTime() const noexcept;

    /**
     * @return The assigned gpio pin number
     */
    [[nodiscard]] gpio_num_t getPinNumber() const noexcept;

private:
    bool isOpen = false;
    bool isInitialized = false;
    GpioDigitalWritePin gpioPin;
    ITime& i_time;
    time_t lastOpenedAt = 0;
};


#endif //IRRIGATION_COMPUTER_TESTS_VALVE_H
