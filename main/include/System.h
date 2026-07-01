//
// Created by Leon Etienne on 18.03.26.
//

#ifndef IRRIGATION_COMPUTER_SYSTEM_H
#define IRRIGATION_COMPUTER_SYSTEM_H

#include "GpioPinRegister.h"
#include "StateMachine.h"
#include "platform/GpioEsp32.h"

/**
 * System entrypoint and main runtime
 */
class System {
public:
    System() noexcept;
    System(const System&) = delete;
    System(System&&) = delete;
    System& operator=(const System&) = delete;

    void Init() noexcept;
    void Update() noexcept;

private:
    StateMachine stateMachine;
    GpioPinRegister gpioPinRegister;
    GpioEsp32 gpio;
};


#endif //IRRIGATION_COMPUTER_SYSTEM_H
