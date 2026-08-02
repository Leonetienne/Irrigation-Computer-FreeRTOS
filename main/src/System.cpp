//
// Created by Leon Etienne on 18.03.26.
//

#include "System.h"
#include "compat/esp_log_macros.h"
#include <array>
#ifndef HOST_BUILD
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

static const char* LOG_TAG = "System";

System::System(
    StateMachine& stateMachine,
    GpioPinRegister& gpioPinRegister,
    IGpio& gpio,
    ITime& i_time,
    INVS& nvs,
    SettingsManager& settings,
    IWifiManager& wifiMan,
    ValveGroup& valveGroup,
    IHttpServer& httpServer
) noexcept :
    stateMachine(stateMachine),
    gpioPinRegister(gpioPinRegister),
    gpio(gpio),
    i_time(i_time),
    nvs(nvs),
    settings(settings),
    wifiMan(wifiMan),
    valveGroup(valveGroup),
    httpServer(httpServer)
{ }

System::~System() noexcept {
    if (isInitialized) {
        free();
    }
}

void System::init() noexcept {
    // wifiMan needs nvs initialized (the wifi driver stores its own state there)
    if (!nvs.begin("system")) {
        ESP_LOGE(LOG_TAG, "nvs.begin failed");
    }

    wifiMan.setOnConnected([this]() { onWifiConnected(); });
    wifiMan.setOnDisconnected([this]() { onWifiDisconnected(); });
    wifiMan.setOnFailed([this]() { onWifiFailed(); });

    const auto storedCredentials = settings.retrieveWifiCredentials();

    if (storedCredentials.has_value()) {
        stateMachine.setState(STATE::WAIT_WIFI_CONNECTION);
        wifiMan.beginUserWifi(*storedCredentials);
    } else {
        // ap comes up immediately, no ip event to wait for
        stateMachine.setState(STATE::WIFI_ONBOARDING);
        wifiMan.beginOnboardingWifi();
        httpServer.begin();
    }

    // no valve is wired up until the user configures count/pins via the advanced settings page
    const int32_t numValves = settings.retrieveNumValves().value_or(0);
    const auto configuredPins = settings.retrieveValveActuatorGpioPins();

    std::array<gpio_num_t, 8> valvePins{};
    for (std::size_t i = 0; i < valvePins.size(); ++i) {
        valvePins[i] = configuredPins.has_value() && static_cast<int32_t>(i) < numValves
            ? (*configuredPins)[i]
            : GPIO_NUM_NC;
    }

    valveGroup.initialize({
        Valve(valvePins[0], gpio, i_time, gpioPinRegister),
        Valve(valvePins[1], gpio, i_time, gpioPinRegister),
        Valve(valvePins[2], gpio, i_time, gpioPinRegister),
        Valve(valvePins[3], gpio, i_time, gpioPinRegister),
        Valve(valvePins[4], gpio, i_time, gpioPinRegister),
        Valve(valvePins[5], gpio, i_time, gpioPinRegister),
        Valve(valvePins[6], gpio, i_time, gpioPinRegister),
        Valve(valvePins[7], gpio, i_time, gpioPinRegister)
    });

    isInitialized = true;
}

void System::loop() noexcept {
    while (stateMachine.getState() != STATE::SHUTTING_DOWN) {
        update();
#ifndef HOST_BUILD
        vTaskDelay(pdMS_TO_TICKS(10));
#endif
    }
    beforeShutdown();
}

bool System::free() noexcept {
    if (!isInitialized) {
        return false;
    }

    // httpServer may already be stopped (e.g. if wifi was disconnected) - that's fine
    httpServer.free();

    if (!wifiMan.free()) {
        return false;
    }

    if (!valveGroup.free()) {
        return false;
    }

    if (!nvs.free()) {
        return false;
    }

    isInitialized = false;
    return true;
}

void System::beforeShutdown() noexcept {
}

void System::update() noexcept {
    valveGroup.autoCloseValvesAfterTimeoutPoll();

    if (wifiConnectFailed) {
        wifiConnectFailed = false;
        ESP_LOGW(LOG_TAG, "wifi connect failed, falling back to onboarding ap");
        httpServer.free();
        wifiMan.free();
        wifiMan.beginOnboardingWifi();
        httpServer.begin();
        stateMachine.setState(STATE::WIFI_ONBOARDING);
    }
}

void System::onWifiConnected() noexcept {
    ESP_LOGI(LOG_TAG, "wifi connected");
    stateMachine.setState(STATE::OPERATIONAL);
    httpServer.begin();
}

void System::onWifiDisconnected() noexcept {
    ESP_LOGW(LOG_TAG, "wifi disconnected");
    stateMachine.setState(STATE::WAIT_WIFI_CONNECTION);

    if (valveGroup.isReady()) {
        if (settings.retrieveCutOnWifiLossEnabled().value_or(true)) {
            for (std::size_t i = 0; i < 8; ++i) {
                if (valveGroup.isValveOperable(i) && valveGroup.getValveOpenState(i)) {
                    valveGroup.close(i);
                }
            }
        }
    }

    httpServer.free();
}

void System::onWifiFailed() noexcept {
    // deferred to update(): must not tear down/rebuild wifi from within its own event callback
    wifiConnectFailed = true;
}
