//
// Created by Leon Etienne on 18.03.26.
//

#ifndef IRRIGATION_COMPUTER_SYSTEM_H
#define IRRIGATION_COMPUTER_SYSTEM_H

#include "GpioPinRegister.h"
#include "StateMachine.h"
#include "SettingsManager.h"
#include "ValveGroup.h"
#include "MqttSync.h"
#include "hal/IGpio.h"
#include "hal/ITime.h"
#include "hal/INVS.h"
#include "hal/IWifiManager.h"
#include "hal/IHttpServer.h"

/**
 * System entrypoint and main runtime.
 *
 * Depends only on interfaces (plus the portable, non-hardware logic classes),
 * so it can be run against either the esp32 platform implementations
 * (see SystemEsp32::getSystem()) or host-side test stubs (see
 * SystemStub::getSystem() / test/System.cpp).
 */
class System {
public:
    System(
        StateMachine& stateMachine,
        GpioPinRegister& gpioPinRegister,
        IGpio& gpio,
        ITime& i_time,
        INVS& nvs,
        SettingsManager& settings,
        IWifiManager& wifiMan,
        ValveGroup& valveGroup,
        IHttpServer& httpServer,
        MqttSync& mqttSync
    ) noexcept;
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

    /**
     * Processes one iteration of runtime work (valve timeout polling, deferred
     * wifi-failure fallback, ...). Called repeatedly by loop().
     */
    void update() noexcept;

private:
    void beforeShutdown() noexcept;

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
    StateMachine& stateMachine;
    GpioPinRegister& gpioPinRegister;
    IGpio& gpio;
    ITime& i_time;
    INVS& nvs;
    SettingsManager& settings;
    IWifiManager& wifiMan;
    ValveGroup& valveGroup;
    IHttpServer& httpServer;
    MqttSync& mqttSync;
};


#endif //IRRIGATION_COMPUTER_SYSTEM_H
