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
#include "SettingsManager.h"
#include "ValveGroup.h"

/**
 * System entrypoint and main runtime
 */
class System {
public:
    System() noexcept;
    System(const System&) = delete;
    System& operator=(const System&) = delete;
    System(System&&) = delete;
    ~System() noexcept;

    void init() noexcept;
    void loop() noexcept;

    /**
     * Will free acquired resources
     * @return Success state
     */
    bool free() noexcept;

private:
    void beforeShutdown() noexcept;
    void update() noexcept;

    /**
     * Called by wifiMan once a connection is established (IP obtained).
     */
    void onWifiConnected() noexcept;

    /**
     * Called by wifiMan once a previously established connection is lost.
     */
    void onWifiDisconnected() noexcept;

    /**
     * Called by wifiMan once connecting with the stored credentials has
     * repeatedly failed. Falls back to onboarding mode.
     */
    void onWifiFailed() noexcept;

    bool isInitialized = false;
    bool wifiConnectFailed = false;
    StateMachine stateMachine;
    GpioPinRegister gpioPinRegister;
    GpioEsp32 gpio;
    TimeEsp32 time;
    NVSEsp32 nvs;
    SettingsManager settings;
    WifiManagerEsp32 wifiMan;
    ValveGroup valveGroup;
    HttpServerEsp32 httpServer;
};


#endif //IRRIGATION_COMPUTER_SYSTEM_H
