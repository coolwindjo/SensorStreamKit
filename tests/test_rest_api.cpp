#include <gtest/gtest.h>
#include "sensorstreamkit/api/rest_server.hpp"
#include <curl/curl.h>
#include <thread>
#include <chrono>

using namespace sensorstreamkit::api;

// Helper to perform HTTP GET using curl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string http_get(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

class RestServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        server_ = std::make_unique<RestServer>(8081, "127.0.0.1");
    }

    void TearDown() override {
        if (server_thread_.joinable()) {
            server_->stop();
            server_thread_.join();
        }
    }

    std::unique_ptr<RestServer> server_;
    std::thread server_thread_;
};

TEST_F(RestServerTest, BasicRoutingAndJson) {
    server_->get("/test", [](const Request&) {
        return Response::json({{"key", "value"}});
    });

    server_thread_ = std::thread([this]() { server_->run(); });
    
    // Wait for server to start
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::string response = http_get("http://127.0.0.1:8081/test");
    auto json = nlohmann::json::parse(response);

    EXPECT_EQ(json["key"], "value");
}

TEST_F(RestServerTest, ErrorHandling) {
    server_->get("/error", [](const Request&) {
        throw std::runtime_error("test error");
        return Response::json({});
    });

    server_thread_ = std::thread([this]() { server_->run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // We can't easily check status code with simple http_get helper, 
    // but we can check if the error message is in the body
    std::string response = http_get("http://127.0.0.1:8081/error");
    auto json = nlohmann::json::parse(response);
    EXPECT_EQ(json["error"], "test error");
}
