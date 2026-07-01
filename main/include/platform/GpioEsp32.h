//
// Created by Leon Etienne on 21.03.26.
//

#ifndef IRRIGATION_COMPUTER_GPIOESP32_H
#define IRRIGATION_COMPUTER_GPIOESP32_H

#include "hal/IGpio.h"

/**
 * Impl of IGpio for esp32
 */
class GpioEsp32 : public IGpio {
public:

    /**
     * Calls gpio_reset_pin
     */
    esp_err_t gpioResetPin(const gpio_num_t pinNum) noexcept override;

    /**
    * Calls gpio_set_direction
    */
    esp_err_t gpioSetDirection(const gpio_num_t pinNum, const gpio_mode_t pinMode) noexcept override;

    /**
     * Calls gpio_set_level
     */
    esp_err_t gpioSetLevel(const gpio_num_t pinNum, const uint32_t level) noexcept override;
};


#endif //IRRIGATION_COMPUTER_GPIOESP32_H
