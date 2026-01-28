#include "sensorstreamkit/api/rest_server.hpp"
#include <iostream>

namespace sensorstreamkit::api {

RestServer::RestServer(int port, std::string host)
    : port_(port), host_(host) {
}

void RestServer::get(const std::string& path, std::function<nlohmann::json(const Request&)> handler) {
    svr_.Get(path, [handler](const httplib::Request& req, httplib::Response& res) {
        try {
            Request api_req{req};
            auto result = handler(api_req);
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            nlohmann::json error = {{"error", e.what()}};
            res.status = 500;
            res.set_content(error.dump(), "application/json");
        }
    });
}

void RestServer::post(const std::string& path, std::function<nlohmann::json(const Request&)> handler) {
    svr_.Post(path, [handler](const httplib::Request& req, httplib::Response& res) {
        try {
            Request api_req{req};
            auto result = handler(api_req);
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            nlohmann::json error = {{"error", e.what()}};
            res.status = 500;
            res.set_content(error.dump(), "application/json");
        }
    });
}

void RestServer::run() {
    std::cout << "[RestServer] Starting server on " << host_ << ":" << port_ << std::endl;
    if (!svr_.listen(host_.c_str(), port_)) {
        std::cerr << "[RestServer] Failed to start server on port " << port_ << std::endl;
    }
}

void RestServer::stop() {
    std::cout << "[RestServer] Stopping server..." << std::endl;
    svr_.stop();
}

bool RestServer::is_running() const {
    return svr_.is_running();
}

} // namespace sensorstreamkit::api
