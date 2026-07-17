//
// Created by Leon Etienne on 18.03.26.
//

#include "System.h"
#include <esp_log.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* LOG_TAG = "System";

System::System() noexcept :
    stateMachine(),
    gpioPinRegister(),
    gpio(),
    time(),
    wifiMan(),
    valveGroup(time)
{ }

System::~System() noexcept {
    if (isInitialized) {
        free();
    }
}

void System::init() noexcept {
    // TODO: temporary until INVS/NVSEsp32/NVSStub exist, then move ownership there
    esp_err_t nvsResult = nvs_flash_init();
    if (nvsResult == ESP_ERR_NVS_NO_FREE_PAGES || nvsResult == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvsResult = nvs_flash_init();
    }
    if (nvsResult != ESP_OK) {
        ESP_LOGE(LOG_TAG, "nvs_flash_init failed: %d", nvsResult);
    }

    wifiMan.setOnConnected([this]() { onWifiConnected(); });
    wifiMan.setOnDisconnected([this]() { onWifiDisconnected(); });

    // Immediately turn on wifi
    wifiMan.begin(
        "bembel2share_optout_nomap",
        "96shork96"
    );

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
}

bool System::free() noexcept {
    if (!isInitialized) {
        return false;
    }

    if (!wifiMan.free()) {
        return false;
    }

    if (!valveGroup.free()) {
        return false;
    }

    isInitialized = false;
    return true;
}

void System::update() noexcept {
}

void System::onWifiConnected() noexcept {
    ESP_LOGI(LOG_TAG, "wifi connected");
}

void System::onWifiDisconnected() noexcept {
    ESP_LOGW(LOG_TAG, "wifi disconnected");
}
