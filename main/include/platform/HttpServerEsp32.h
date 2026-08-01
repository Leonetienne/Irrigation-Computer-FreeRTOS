#ifndef IRRIGATION_COMPUTER_TESTS_HTTPSERVERESP32_H
#define IRRIGATION_COMPUTER_TESTS_HTTPSERVERESP32_H

#include "esp_http_server.h"
#include "ValveGroup.h"
#include "SettingsManager.h"
#include "StateMachine.h"

/**
 * Esp32-Implementation of the web ui / api http server.
 */
class HttpServerEsp32 {
public:
    HttpServerEsp32(ValveGroup& valveGroup, SettingsManager& settings, StateMachine& stateMachine) noexcept;
    HttpServerEsp32(const HttpServerEsp32&) = delete;
    HttpServerEsp32& operator=(const HttpServerEsp32&) = delete;
    HttpServerEsp32(HttpServerEsp32&&) = delete;
    HttpServerEsp32& operator=(HttpServerEsp32&&) = delete;
    ~HttpServerEsp32() noexcept;

    /**
     * Starts the http server and registers all uri handlers
     * @return Success state
     */
    bool begin() noexcept;

    /**
     * Stops the http server and releases the resources acquired by begin()
     * @return Success state
     */
    bool free() noexcept;

private:
    /**
     * Serves the embedded web ui (index.html)
     */
    static esp_err_t handleGetFile(httpd_req_t* req) noexcept;

    /**
     * Routes GET /api/ requests
     * TODO: no GET api routes defined yet
     */
    static esp_err_t handleGetApi(httpd_req_t* req) noexcept;

    /**
     * Routes POST /api/ requests
     */
    static esp_err_t handlePost(httpd_req_t* req) noexcept;

    /**
     * Parses and dispatches a valve command
     */
    static esp_err_t handleValveCommand(httpd_req_t* req) noexcept;

    /**
     * Saves wifi credentials submitted via the web ui and requests a shutdown
     */
    static esp_err_t handleWifiCredentials(httpd_req_t* req) noexcept;

    bool isInitialized = false;
    httpd_handle_t server = nullptr;
    ValveGroup& valveGroup;
    SettingsManager& settings;
    StateMachine& stateMachine;
};

#endif //IRRIGATION_COMPUTER_TESTS_HTTPSERVERESP32_H
