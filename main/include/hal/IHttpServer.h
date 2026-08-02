#ifndef IRRIGATION_COMPUTER_TESTS_IHTTPSERVER_H
#define IRRIGATION_COMPUTER_TESTS_IHTTPSERVER_H

/**
 * Abstract interface for the web ui / api http server
 */
class IHttpServer {
public:
    IHttpServer() = default;
    IHttpServer(const IHttpServer&) = delete;
    IHttpServer& operator=(const IHttpServer&) = delete;
    IHttpServer(IHttpServer&&) = delete;
    IHttpServer& operator=(IHttpServer&&) = delete;
    virtual ~IHttpServer() = default;

    /**
     * Starts the http server and registers all uri handlers
     * @return Success state
     */
    virtual bool begin() noexcept = 0;

    /**
     * Stops the http server and releases the resources acquired by begin()
     * @return Success state
     */
    virtual bool free() noexcept = 0;
};

#endif //IRRIGATION_COMPUTER_TESTS_IHTTPSERVER_H
