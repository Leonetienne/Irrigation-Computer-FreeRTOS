//
// Created by Leon Etienne on 18.03.26.
//

#include "System.h"
#include <esp_log.h>
#include <array>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* LOG_TAG = "System";

System::System() noexcept :
    stateMachine(),
    gpioPinRegister(),
    gpio(),
    time(),
    nvs(),
    settings(nvs),
    wifiMan(),
    valveGroup(time),
    httpServer(valveGroup, settings, stateMachine)
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
        Valve(valvePins[0], gpio, time, gpioPinRegister),
        Valve(valvePins[1], gpio, time, gpioPinRegister),
        Valve(valvePins[2], gpio, time, gpioPinRegister),
        Valve(valvePins[3], gpio, time, gpioPinRegister),
        Valve(valvePins[4], gpio, time, gpioPinRegister),
        Valve(valvePins[5], gpio, time, gpioPinRegister),
        Valve(valvePins[6], gpio, time, gpioPinRegister),
        Valve(valvePins[7], gpio, time, gpioPinRegister)
    });

    isInitialized = true;
}

void System::loop() noexcept {
    while (stateMachine.getState() != STATE::SHUTTING_DOWN) {
        update();
        vTaskDelay(pdMS_TO_TICKS(10));
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
    httpServer.free();
}

void System::onWifiFailed() noexcept {
    // deferred to update(): must not tear down/rebuild wifi from within its own event callback
    wifiConnectFailed = true;
}
