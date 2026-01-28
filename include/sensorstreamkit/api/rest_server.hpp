#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <string>
#include <functional>
#include <map>

namespace sensorstreamkit::api {

/**
 * @brief Wrapper for httplib::Request to provide a cleaner interface
 */
struct Request {
    const httplib::Request& raw_request;
    
    std::string get_param(const std::string& key) const {
        return raw_request.get_param_value(key);
    }
};

/**
 * @brief Utility for creating JSON responses
 */
struct Response {
    static nlohmann::json json(const nlohmann::json& data) {
        return data;
    }
};

/**
 * @brief Lightweight REST server for configuration and monitoring
 */
class RestServer {
public:
    explicit RestServer(int port = 8080, std::string host = "0.0.0.0");
    ~RestServer() = default;

    // Delete copy constructor and assignment
    RestServer(const RestServer&) = delete;
    RestServer& operator=(const RestServer&) = delete;

    /**
     * @brief Register a GET endpoint
     */
    void get(const std::string& path, std::function<nlohmann::json(const Request&)> handler);

    /**
     * @brief Register a POST endpoint
     */
    void post(const std::string& path, std::function<nlohmann::json(const Request&)> handler);

    /**
     * @brief Start the server (blocking)
     */
    void run();

    /**
     * @brief Stop the server
     */
    void stop();

    /**
     * @brief Check if the server is running
     */
    bool is_running() const;

private:
    int port_;
    std::string host_;
    httplib::Server svr_;
};

} // namespace sensorstreamkit::api
