#pragma once

/**
 * @file zmq_publisher.hpp
 * @brief ZeroMQ PUB socket wrapper with modern C++20 patterns
 * @author Jo, SeungHyeon (Jo,SH)
 */

#include <zmq.hpp>
#include <string>
#include <string_view>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <optional>

#include "sensorstreamkit/core/message.hpp"

using namespace sensorstreamkit::core;

namespace sensorstreamkit::transport {

/**
 * @brief Configuration for ZeroMQ publisher
 */
struct PublisherConfig {
    std::string endpoint = "tcp://*:5555";
    int high_water_mark = 1000;
    int send_timeout_ms = 1000;
    bool conflate = false;  // Keep only last message per topic
};

/**
 * @brief ZeroMQ PUB socket wrapper with RAII and type safety
 */
class ZmqPublisher {
public:
    explicit ZmqPublisher(const PublisherConfig& config = {});
    ~ZmqPublisher();

    // Non-copyable, movable
    ZmqPublisher(const ZmqPublisher&) = delete;
    ZmqPublisher& operator=(const ZmqPublisher&) = delete;
    ZmqPublisher(ZmqPublisher&&) noexcept;
    ZmqPublisher& operator=(ZmqPublisher&&) noexcept;

    /**
     * @brief Bind to endpoint
     * @return true if successful
     */
    [[nodiscard]] bool bind();

    /**
     * @brief Connect to endpoint
     * @return true if successful
     */
    [[nodiscard]] bool connect();

    /**
     * @brief Publish a message with topic
     * @tparam T Message payload type (must satisfy SensorDataType concept)
     * @param topic Topic string for subscribers to filter
     * @param message The message to publish
     * @return true if sent successfully
     */
    template <SensorDataType T>
    bool publish(std::string_view topic, const Message<T>& message) {
        std::vector<uint8_t> buffer;
        message.serialize(buffer);
        return publish_raw(topic, buffer);
    }

    /**
     * @brief Publish raw bytes with topic
     */
    bool publish_raw(std::string_view topic, std::span<const uint8_t> data);

    /**
     * @brief Get total messages sent
     */
    [[nodiscard]] uint64_t messages_sent() const noexcept {
        return messages_sent_.load();
    }

private:
    PublisherConfig config_;
    std::unique_ptr<zmq::context_t> context_;
    std::unique_ptr<zmq::socket_t> socket_;
    std::atomic<uint64_t> messages_sent_{0};
    std::atomic<bool> bound_{false};
};

/**
 * @brief Periodic sensor publisher using std::jthread (C++20)
 */
template <SensorDataType T>
class PeriodicPublisher {
public:
    using DataGenerator = std::function<T()>;

    PeriodicPublisher(ZmqPublisher& publisher,
                      std::string topic,
                      DataGenerator generator,
                      std::chrono::milliseconds interval)
        : publisher_(publisher)
        , topic_(std::move(topic))
        , generator_(std::move(generator))
        , interval_(interval) {}

    /**
     * @brief Start periodic publishing in a separate thread
     */
    void start() {
        worker_thread_ = std::jthread([this](std::stop_token stoken) {
            while (!stoken.stop_requested()) {
                T data = generator_();
                Message<T> message(data);
                publisher_.publish<T>(topic_, message);
                std::this_thread::sleep_for(interval_);
            }
        });
    }

    /**
     * @brief Stop periodic publishing
     */
    void stop() {
        if (worker_thread_.joinable()) {
            worker_thread_.request_stop();
            worker_thread_.join();
        }
    }

    /**
     * @brief Check if publisher is running
     */
    [[nodiscard]] bool is_running() const noexcept {
        return worker_thread_.joinable() && !worker_thread_.get_stop_token().stop_requested();
    }

private:
    ZmqPublisher& publisher_;
    std::string topic_;
    DataGenerator generator_;
    std::chrono::milliseconds interval_;
    std::jthread worker_thread_;
};

}   // namespace sensorstreamkit::transport