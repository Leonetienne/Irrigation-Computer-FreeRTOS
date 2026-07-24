//
// Created by Leon Etienne on 18.03.26.
//

#include "System.h"
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* LOG_TAG = "System";

System::System() noexcept :
    stateMachine(),
    gpioPinRegister(),
    gpio(),
    time(),
    nvs(),
    wifiMan(),
    valveGroup(time),
    httpServer(valveGroup, nvs, stateMachine)
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

    char storedSsid[NVS_MAX_STRING_LENGTH + 1] = {};
    char storedPassword[NVS_MAX_STRING_LENGTH + 1] = {};
    const bool hasStoredCredentials =
        nvs.getString("wifi_ssid", storedSsid) &&
        nvs.getString("wifi_pass", storedPassword);

    if (hasStoredCredentials) {
        wifiMan.beginUserWifi(WifiCredentials{storedSsid, storedPassword});
    } else {
        // ap comes up immediately, no ip event to wait for
        wifiMan.beginOnboardingWifi();
        httpServer.begin();
    }

    // Load valves (four valves active, four inactive)
    valveGroup.initialize({
        Valve(GPIO_NUM_13, gpio, time, gpioPinRegister),
        Valve(GPIO_NUM_14, gpio, time, gpioPinRegister),
        Valve(GPIO_NUM_15, gpio, time, gpioPinRegister),
        Valve(GPIO_NUM_16, gpio, time, gpioPinRegister),
        Valve(GPIO_NUM_NC, gpio, time, gpioPinRegister),
        Valve(GPIO_NUM_NC, gpio, time, gpioPinRegister),
        Valve(GPIO_NUM_NC, gpio, time, gpioPinRegister),
        Valve(GPIO_NUM_NC, gpio, time, gpioPinRegister)
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
}

void System::onWifiConnected() noexcept {
    ESP_LOGI(LOG_TAG, "wifi connected");
    httpServer.begin();
}

void System::onWifiDisconnected() noexcept {
    ESP_LOGW(LOG_TAG, "wifi disconnected");
    httpServer.free();
}
