#ifndef IRRIGATION_COMPUTER_TESTS_HTTPSERVERESP32_H
#define IRRIGATION_COMPUTER_TESTS_HTTPSERVERESP32_H

#include "esp_http_server.h"
#include "ValveGroup.h"

/**
 * Esp32-Implementation of the web ui / api http server.
 */
class HttpServerEsp32 {
public:
    HttpServerEsp32() = default;
    ~HttpServerEsp32() noexcept;

    HttpServerEsp32(const HttpServerEsp32&) = delete;
    HttpServerEsp32& operator=(const HttpServerEsp32&) = delete;
    HttpServerEsp32(HttpServerEsp32&&) = delete;
    HttpServerEsp32& operator=(HttpServerEsp32&&) = delete;

    /**
     * Starts the http server and registers all uri handlers
     * @param valveGroup operated on by valve api commands, must outlive this server
     * @return Success state
     */
    bool begin(ValveGroup& valveGroup) noexcept;

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
     * Routes POST /api/ requests. Parses the route via ApiRouteParser
     * and dispatches the resulting command via ApiController.
     */
    static esp_err_t handlePost(httpd_req_t* req) noexcept;

    bool isInitialized = false;
    httpd_handle_t server = nullptr;
    ValveGroup* valveGroup = nullptr;
};

#endif //IRRIGATION_COMPUTER_TESTS_HTTPSERVERESP32_H
