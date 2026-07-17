#include "platform/WifiManagerEsp32.h"

#include <esp_log.h>
#include "esp_wifi.h"
#include <cstring>

static const char* LOG_TAG = "WifiManagerEsp32";

WifiManagerEsp32::~WifiManagerEsp32() noexcept {
    if (isInitialized) {
        WifiManagerEsp32::free();
    }
}

bool WifiManagerEsp32::begin(const char* ssid, const char* password) noexcept {
    if (isInitialized) {
        return false;
    }

    if (esp_netif_init() != ESP_OK) {
        return false;
    }
    if (esp_event_loop_create_default() != ESP_OK) {
        return false;
    }
    staNetif = esp_netif_create_default_wifi_sta();
    if (staNetif == nullptr) {
        return false;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK) {
        return false;
    }

    if (
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &WifiManagerEsp32::eventHandler,
            this
        ) != ESP_OK) {
        return false;
    }
    if (
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &WifiManagerEsp32::eventHandler,
            this
        ) != ESP_OK) {
        return false;
    }

    wifi_config_t wifiConfig = {};
    std::strncpy(
        reinterpret_cast<char *>(wifiConfig.sta.ssid),
        ssid,
        sizeof(wifiConfig.sta.ssid) - 1
    );
    std::strncpy(
        reinterpret_cast<char *>(wifiConfig.sta.password),
        password,
        sizeof(wifiConfig.sta.password) - 1
    );
    wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK; // Use at min WPA2

    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
        return false;
    }
    if (esp_wifi_set_config(WIFI_IF_STA, &wifiConfig) != ESP_OK) {
        return false;
    }
    if (esp_wifi_start() != ESP_OK) {
        return false;
    }

    isInitialized = true;
    return true;
}

bool WifiManagerEsp32::free() noexcept {
    if (!isInitialized) {
        return false;
    }

    bool success = true;

    if (esp_wifi_stop() != ESP_OK) {
        success = false;
    }
    if (esp_event_handler_unregister(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiManagerEsp32::eventHandler) != ESP_OK) {
        success = false;
    }
    if (esp_event_handler_unregister(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiManagerEsp32::eventHandler) != ESP_OK) {
        success = false;
    }
    if (esp_wifi_deinit() != ESP_OK) {
        success = false;
    }

    if (staNetif != nullptr) {
        esp_netif_destroy_default_wifi(staNetif);
        staNetif = nullptr;
    }

    if (esp_event_loop_delete_default() != ESP_OK) {
        success = false;
    }

    isInitialized = false;
    state = WifiConnectionState::Disconnected;

    return success;
}

WifiConnectionState WifiManagerEsp32::getState() const noexcept {
    return state;
}

void WifiManagerEsp32::setOnConnected(std::function<void()> callback) noexcept {
    onConnected = std::move(callback);
}

void WifiManagerEsp32::setOnDisconnected(std::function<void()> callback) noexcept {
    onDisconnected = std::move(callback);
}

void WifiManagerEsp32::eventHandler(
    void* arg,
    esp_event_base_t base,
    int32_t id,
    void* data
) noexcept {
    auto* self = static_cast<WifiManagerEsp32*>(arg);

    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        self->state = WifiConnectionState::Connecting;
        esp_wifi_connect();

    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        const bool wasConnected = self->state == WifiConnectionState::Connected;

        self->state = WifiConnectionState::Connecting;
        ESP_LOGW(LOG_TAG, "disconnected, retrying");
        esp_wifi_connect(); // Immediate non-blocking retry

        if (wasConnected && self->onDisconnected) {
            self->onDisconnected();
        }

    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        self->state = WifiConnectionState::Connected;
        ESP_LOGI(LOG_TAG, "connected, got ip");

        if (self->onConnected) {
            self->onConnected();
        }
    }
}
