//
// Created by Leon Etienne on 18.03.26.
//

#ifndef IRRIGATION_COMPUTER_SYSTEM_H
#define IRRIGATION_COMPUTER_SYSTEM_H

#include "GpioPinRegister.h"
#include "StateMachine.h"
#include "platform/GpioEsp32.h"
#include "platform/TimeEsp32.h"
#include "platform/WifiManagerEsp32.h"
#include "platform/HttpServerEsp32.h"
#include "platform/NVSEsp32.h"
#include "ValveGroup.h"

/**
 * System entrypoint and main runtime
 */
class System {
public:
    System() noexcept;
    System(const System&) = delete;
    System(System&&) = delete;
    ~System() noexcept;
    System& operator=(const System&) = delete;

    void init() noexcept;
    void loop() noexcept;

    /**
     * Will free acquired resources
     * @return Success state
     */
    bool free() noexcept;

private:
    void update() noexcept;

    /**
     * Called by wifiMan once a connection is established (IP obtained).
     */
    void onWifiConnected() noexcept;

    /**
     * Called by wifiMan once a previously established connection is lost.
     */
    void onWifiDisconnected() noexcept;

    bool isInitialized = false;
    StateMachine stateMachine;
    GpioPinRegister gpioPinRegister;
    GpioEsp32 gpio;
    TimeEsp32 time;
    NVSEsp32 nvs;
    WifiManagerEsp32 wifiMan;
    HttpServerEsp32 httpServer;
    ValveGroup valveGroup;
};


#endif //IRRIGATION_COMPUTER_SYSTEM_H
