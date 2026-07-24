#include "platform/HttpServerEsp32.h"
#include "ApiRouteParser.h"
#include "ApiController.h"
#include "WifiCredentialsParser.h"

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

HttpServerEsp32::HttpServerEsp32(ValveGroup& valveGroup, INVS& nvs, StateMachine& stateMachine) noexcept:
    valveGroup(valveGroup),
    nvs(nvs),
    stateMachine(stateMachine)
{ }

HttpServerEsp32::~HttpServerEsp32() noexcept {
    if (isInitialized) {
        HttpServerEsp32::free();
    }
}

bool HttpServerEsp32::begin() noexcept {
    if (isInitialized) {
        return false;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server, &config) != ESP_OK) {
        return false;
    }

    static const httpd_uri_t getFileUri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = handleGetFile,
        .user_ctx = nullptr,
    };
    static const httpd_uri_t getApiUri = {
        .uri = "/api/*",
        .method = HTTP_GET,
        .handler = handleGetApi,
        .user_ctx = this,
    };
    static const httpd_uri_t postApiUri = {
        .uri = "/api/*",
        .method = HTTP_POST,
        .handler = handlePost,
        .user_ctx = this,
    };

    if (httpd_register_uri_handler(server, &getFileUri) != ESP_OK) {
        return false;
    }
    if (httpd_register_uri_handler(server, &getApiUri) != ESP_OK) {
        return false;
    }
    if (httpd_register_uri_handler(server, &postApiUri) != ESP_OK) {
        return false;
    }

    isInitialized = true;
    return true;
}

bool HttpServerEsp32::free() noexcept {
    if (!isInitialized) {
        return false;
    }

    if (httpd_stop(server) != ESP_OK) {
        return false;
    }

    server = nullptr;
    isInitialized = false;
    return true;
}

esp_err_t HttpServerEsp32::handleGetFile(httpd_req_t* req) noexcept {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(
        req,
        reinterpret_cast<const char*>(index_html_start),
        static_cast<ssize_t>(index_html_end - index_html_start)
    );
}

esp_err_t HttpServerEsp32::handleGetApi(httpd_req_t* req) noexcept {
    // TODO: no GET api routes defined yet
    httpd_resp_set_status(req, "501 Not Implemented");
    httpd_resp_send(req, nullptr, 0);
    return ESP_OK;
}

esp_err_t HttpServerEsp32::handlePost(httpd_req_t* req) noexcept {
    const std::string_view uri = req->uri;

    if (uri == "/api/wifi") {
        return handleWifiCredentials(req);
    }

    return handleValveCommand(req);
}

esp_err_t HttpServerEsp32::handleValveCommand(httpd_req_t* req) noexcept {
    const auto command = ApiRouteParser::parseValveRoute(req->uri);
    if (!command.has_value()) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    auto* self = static_cast<HttpServerEsp32*>(req->user_ctx);
    if (!ApiController::executeValveOperation(self->valveGroup, *command)) {
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, nullptr, 0);
        return ESP_OK;
    }

    httpd_resp_send(req, nullptr, 0);
    return ESP_OK;
}

esp_err_t HttpServerEsp32::handleWifiCredentials(httpd_req_t* req) noexcept {
    char body[256] = {};
    const int contentLength = req->content_len < sizeof(body) - 1
        ? static_cast<int>(req->content_len)
        : static_cast<int>(sizeof(body) - 1);

    const int received = httpd_req_recv(req, body, contentLength);
    if (received <= 0) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, nullptr, 0);
        return ESP_FAIL;
    }
    body[received] = '\0';

    const auto credentials = WifiCredentialsParser::parse(std::string_view(body, received));
    if (!credentials.has_value()) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, nullptr, 0);
        return ESP_FAIL;
    }

    auto* self = static_cast<HttpServerEsp32*>(req->user_ctx);
    if (!ApiController::saveWifiCredentials(self->nvs, self->stateMachine, *credentials)) {
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, nullptr, 0);
        return ESP_OK;
    }

    httpd_resp_send(req, nullptr, 0);
    return ESP_OK;
}
