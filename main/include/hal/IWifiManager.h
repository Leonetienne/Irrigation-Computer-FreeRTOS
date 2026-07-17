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
     * @return Success state
     */
    virtual bool begin(const char* ssid, const char* password) noexcept = 0;

    /**
     * Will release the resources acquired by begin()
     * @return Success state
     */
    virtual bool free() noexcept = 0;

    /**
     * @return The wifi session state
     */
    [[nodiscard]] virtual WifiConnectionState getState() const noexcept = 0;

    /**
     * Callback setter
     * @param callback
     */
    virtual void setOnConnected(std::function<void()> callback) noexcept = 0;
    /**
     * Callback setter
     * @param callback
     */
    virtual void setOnDisconnected(std::function<void()> callback) noexcept = 0;
};

#endif //IRRIGATION_COMPUTER_TESTS_IWIFIMANAGERS_H
