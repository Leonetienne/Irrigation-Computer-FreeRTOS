#ifndef IRRIGATION_COMPUTER_TESTS_IWIFIMANAGERS_H
#define IRRIGATION_COMPUTER_TESTS_IWIFIMANAGERS_H

#include <functional>

enum class WifiConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Failed,
};

/**
 * Abstract interface to manage a wifi connection
 */
class IWifiManager {
public:
    virtual ~IWifiManager() = default;

    /**
     * Will begin the wifi session
     * @param ssid
     * @param password
     */
    virtual void begin(const char* ssid, const char* password) = 0;

    /**
     * @return The wifi session state
     */
    [[nodiscard]] virtual WifiConnectionState getState() const = 0;

    /**
     * Callback setter
     * @param callback
     */
    virtual void setOnConnected(std::function<void()> callback) = 0;
    /**
     * Callback setter
     * @param callback
     */
    virtual void setOnDisconnected(std::function<void()> callback) = 0;
};

#endif //IRRIGATION_COMPUTER_TESTS_IWIFIMANAGERS_H
