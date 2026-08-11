#include "platform/WifiManagerEsp32.h"

#include <esp_log.h>
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_random.h"
#include <cstring>
#include <cstdio>

static const char* LOG_TAG = "WifiManagerEsp32";

WifiManagerEsp32::WifiManagerEsp32(IGpio& gpio, GpioPinRegister& pinRegister, const ITime& i_time) noexcept :
    gpio(gpio),
    pinRegister(pinRegister),
    i_time(i_time)
{ }

WifiManagerEsp32::~WifiManagerEsp32() noexcept {
    if (isInitialized) {
        WifiManagerEsp32::free();
    }
}

bool WifiManagerEsp32::beginUserWifi(const WifiCredentials& credentials) noexcept {
    if (isInitialized) {
        return false;
    }

    connectFailureCount = 0;

    if (esp_netif_init() != ESP_OK) {
        return false;
    }
    if (esp_event_loop_create_default() != ESP_OK) {
        return false;
    }
    netif = esp_netif_create_default_wifi_sta();
    if (netif == nullptr) {
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
    eventHandlersRegistered = true;

    wifi_config_t wifiConfig = {};
    std::strncpy(
        reinterpret_cast<char *>(wifiConfig.sta.ssid),
        credentials.ssid.c_str(),
        sizeof(wifiConfig.sta.ssid) - 1
    );
    std::strncpy(
        reinterpret_cast<char *>(wifiConfig.sta.password),
        credentials.password.c_str(),
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

bool WifiManagerEsp32::beginOnboardingWifi() noexcept {
    if (isInitialized) {
        return false;
    }

    if (esp_netif_init() != ESP_OK) {
        return false;
    }
    if (esp_event_loop_create_default() != ESP_OK) {
        return false;
    }
    netif = esp_netif_create_default_wifi_ap();
    if (netif == nullptr) {
        return false;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK) {
        return false;
    }

    char ssid[32] = {};
    std::snprintf(
        ssid,
        sizeof(ssid),
        "irrigation_computer_%06u",
        static_cast<unsigned>(esp_random() % 1000000)
    );

    wifi_config_t wifiConfig = {};
    std::strncpy(
        reinterpret_cast<char *>(wifiConfig.ap.ssid),
        ssid,
        sizeof(wifiConfig.ap.ssid) - 1
    );
    wifiConfig.ap.ssid_len = std::strlen(ssid);
    wifiConfig.ap.channel = 1;
    wifiConfig.ap.authmode = WIFI_AUTH_OPEN;
    wifiConfig.ap.max_connection = 4;

    if (esp_wifi_set_mode(WIFI_MODE_AP) != ESP_OK) {
        return false;
    }
    if (esp_wifi_set_config(WIFI_IF_AP, &wifiConfig) != ESP_OK) {
        return false;
    }
    if (esp_wifi_start() != ESP_OK) {
        return false;
    }

    ESP_LOGI(LOG_TAG, "onboarding ap started, ssid: %s", ssid);

    // the ap is up as soon as esp_wifi_start succeeds
    state = WifiConnectionState::Connected;
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
    if (eventHandlersRegistered) {
        if (esp_event_handler_unregister(
            WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiManagerEsp32::eventHandler) != ESP_OK) {
            success = false;
        }
        if (esp_event_handler_unregister(
            IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiManagerEsp32::eventHandler) != ESP_OK) {
            success = false;
        }
    }
    if (esp_wifi_deinit() != ESP_OK) {
        success = false;
    }

    if (netif != nullptr) {
        esp_netif_destroy_default_wifi(netif);
        netif = nullptr;
    }

    if (esp_event_loop_delete_default() != ESP_OK) {
        success = false;
    }

    isInitialized = false;
    eventHandlersRegistered = false;
    state = WifiConnectionState::Disconnected;
    connectFailureCount = 0;
    setIndicatorState(PIN_STATE_DIGITAL::LOW);

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

void WifiManagerEsp32::setOnFailed(std::function<void()> callback) noexcept {
    onFailed = std::move(callback);
}

bool WifiManagerEsp32::setIndicatorGpioPin(gpio_num_t pin) noexcept {
    if (indicatorPin.has_value()) {
        return false;
    }
    if (pin == GPIO_NUM_NC) {
        return true;
    }

    indicatorPin.emplace(pinRegister, gpio, pin);
    if (!indicatorPin->initialize()) {
        indicatorPin.reset();
        return false;
    }

    indicatorPin->setState(PIN_STATE_DIGITAL::LOW);
    lastBlinkToggleAtMs = i_time.getMillis();
    return true;
}

void WifiManagerEsp32::updateOnboardingModeLedBlink() noexcept {
    if (!indicatorPin.has_value() || !indicatorPin->isReady()) {
        return;
    }

    const int64_t now = i_time.getMillis();
    if (now - lastBlinkToggleAtMs < BLINK_INTERVAL_MS) {
        return;
    }

    lastBlinkToggleAtMs = now;
    setIndicatorState(
        indicatorPin->getState() == PIN_STATE_DIGITAL::HIGH ? PIN_STATE_DIGITAL::LOW : PIN_STATE_DIGITAL::HIGH
    );
}

void WifiManagerEsp32::setIndicatorState(PIN_STATE_DIGITAL pinState) noexcept {
    if (indicatorPin.has_value()) {
        indicatorPin->setState(pinState);
    }
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
        self->setIndicatorState(PIN_STATE_DIGITAL::LOW);
        const bool wasConnected = self->state == WifiConnectionState::Connected;

        if (wasConnected) {
            // was connected before, then dropped out - keep retrying indefinitely
            self->connectFailureCount = 0;
            self->state = WifiConnectionState::Connecting;
            ESP_LOGW(LOG_TAG, "disconnected, retrying");
            esp_wifi_connect();

            if (self->onDisconnected) {
                self->onDisconnected();
            }
            return;
        }

        ++self->connectFailureCount;
        if (self->connectFailureCount >= MAX_CONNECT_RETRIES) {
            ESP_LOGW(LOG_TAG, "giving up after %d failed connection attempts", self->connectFailureCount);
            self->state = WifiConnectionState::Failed;

            if (self->onFailed) {
                self->onFailed();
            }
        } else {
            self->state = WifiConnectionState::Connecting;
            ESP_LOGW(LOG_TAG, "connect failed, retrying (%d/%d)", self->connectFailureCount, MAX_CONNECT_RETRIES);
            esp_wifi_connect();
        }

    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        self->state = WifiConnectionState::Connected;
        self->connectFailureCount = 0;
        self->setIndicatorState(PIN_STATE_DIGITAL::HIGH);
        const auto* event = static_cast<ip_event_got_ip_t*>(data);
        ESP_LOGI(LOG_TAG, "connected, got ip: " IPSTR, IP2STR(&event->ip_info.ip));

        if (self->onConnected) {
            self->onConnected();
        }
    }
}
