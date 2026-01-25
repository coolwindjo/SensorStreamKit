#pragma once

/**
 * @file zmq_subscriber.hpp
 * @brief ZeroMQ SUB socket wrapper with modern C++20 patterns
 * @author Jo, SeungHyeon (Jo,SH)
 */

#include <zmq.hpp>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <optional>
#include <atomic>
#include <stop_token>
#include <unordered_set>

#include "sensorstreamkit/core/message.hpp"

using namespace sensorstreamkit::core;

namespace sensorstreamkit::transport {

/**
 * @brief Configuration for ZeroMQ subscriber
 */
struct SubscriberConfig {
    std::string endpoint = "tcp://localhost:5555";
    int high_water_mark = 1000;
    int receive_timeout_ms = 1000;
    bool conflate = false;  // Keep only last message per topic
};

/**
 * @brief ZeroMQ SUB socket wrapper with RAII and type safety
 */
class ZmqSubscriber {
public:
    explicit ZmqSubscriber(const SubscriberConfig& config = {});
    ~ZmqSubscriber();

    // Non-copyable, movable
    ZmqSubscriber(const ZmqSubscriber&) = delete;
    ZmqSubscriber& operator=(const ZmqSubscriber&) = delete;
    ZmqSubscriber(ZmqSubscriber&&) noexcept;
    ZmqSubscriber& operator=(ZmqSubscriber&&) noexcept;

    /**
     * @brief Connect to endpoint
     * @return true if successful
     */
    [[nodiscard]] bool connect();

    /**
     * @brief Subscribe to a topic (empty string = subscribe to all)
     * @param topic Topic filter
     * @return true if successful
     */
    bool subscribe(std::string_view topic = "");

    /**
     * @brief Unsubscribe from a topic
     * @param topic Topic filter
     * @return true if successful
     */
    bool unsubscribe(std::string_view topic);

    /**
     * @brief Receive a message with topic
     * @tparam T Message payload type (must satisfy SensorDataType concept)
     * @param topic Output topic string
     * @param message Output message
     * @return true if received successfully
     */
    template <SensorDataType T>
    [[nodiscard]] std::optional<Message<T>> receive(std::stop_token stoken = {}) {
        auto data = receive_raw(stoken);
        if (!data) {
            return std::nullopt;
        }
        return Message<T>::deserialize(*data);
    }

    /**
     * @brief Receive raw bytes with topic
     * @param topic Output topic string
     * @param data Output data buffer
     * @return true if received successfully
     */
    [[nodiscard]] std::optional<std::vector<uint8_t>> receive_raw(std::stop_token stoken = {});

    /**
     * @brief Get total messages received
     */
    [[nodiscard]] uint64_t messages_received() const noexcept {
        return messages_received_.load();
    }

    /**
     * @brief Check if connected
     */
    [[nodiscard]] bool is_connected() const noexcept {
        return connected_.load();
    }

private:
    SubscriberConfig config_;
    std::unique_ptr<zmq::context_t> context_;
    std::unique_ptr<zmq::socket_t> socket_;
    std::atomic<uint64_t> messages_received_{0};
    std::atomic<bool> connected_{false};
    std::unordered_set<std::string> subscriptions_;
};

}   // namespace sensorstreamkit::transport
