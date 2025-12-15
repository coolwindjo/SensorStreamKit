/**
 * @file zmq_subscriber.cpp
 * @brief ZeroMQ subscriber implementation
 */

#include "sensorstreamkit/transport/zmq_subscriber.hpp"
#include <stdexcept>

namespace sensorstreamkit::transport {

ZmqSubscriber::ZmqSubscriber(const SubscriberConfig& config)
    : config_(config)
    , context_(std::make_unique<zmq::context_t>(1))
    , socket_(std::make_unique<zmq::socket_t>(*context_, zmq::socket_type::sub)) {

    socket_->set(zmq::sockopt::rcvhwm, config_.high_water_mark);
    socket_->set(zmq::sockopt::rcvtimeo, config_.receive_timeout_ms);
}

ZmqSubscriber::~ZmqSubscriber() {
    if (socket_) {
        socket_->close();
    }
    if (context_) {
        context_->close();
    }
}

ZmqSubscriber::ZmqSubscriber(ZmqSubscriber&& other) noexcept
    : config_(std::move(other.config_))
    , context_(std::move(other.context_))
    , socket_(std::move(other.socket_))
    , messages_received_(other.messages_received_.load())
    , connected_(other.connected_.load()) {
}

ZmqSubscriber& ZmqSubscriber::operator=(ZmqSubscriber&& other) noexcept {
    if (this != &other) {
        config_ = std::move(other.config_);
        context_ = std::move(other.context_);
        socket_ = std::move(other.socket_);
        messages_received_ = other.messages_received_.load();
        connected_ = other.connected_.load();
    }
    return *this;
}

bool ZmqSubscriber::connect() {
    try {
        socket_->connect(config_.endpoint);
        connected_ = true;
        return true;
    } catch (const zmq::error_t& e) {
        connected_ = false;
        return false;
    }
}

bool ZmqSubscriber::subscribe(std::string_view topic) {
    if (!connected_) {
        return false;
    }

    try {
        socket_->set(zmq::sockopt::subscribe, topic);
        return true;
    } catch (const zmq::error_t& e) {
        return false;
    }
}

bool ZmqSubscriber::unsubscribe(std::string_view topic) {
    if (!connected_) {
        return false;
    }

    try {
        socket_->set(zmq::sockopt::unsubscribe, topic);
        return true;
    } catch (const zmq::error_t& e) {
        return false;
    }
}

std::optional<std::vector<uint8_t>> ZmqSubscriber::receive_raw() {
    if (!connected_) {
        return std::nullopt;
    }

    try {
        // Receive topic (first part of multipart message)
        zmq::message_t topic_msg;
        auto result = socket_->recv(topic_msg, zmq::recv_flags::none);
        if (!result) {
            return std::nullopt;  // Timeout or error
        }

        // Receive data (second part of multipart message)
        zmq::message_t data_msg;
        result = socket_->recv(data_msg, zmq::recv_flags::none);
        if (!result) {
            return std::nullopt;  // Timeout or error
        }

        std::vector<uint8_t> data(
            static_cast<uint8_t*>(data_msg.data()),
            static_cast<uint8_t*>(data_msg.data()) + data_msg.size()
        );

        messages_received_.fetch_add(1, std::memory_order_relaxed);
        return data;
    } catch (const zmq::error_t& e) {
        return std::nullopt;
    }
}

} // namespace sensorstreamkit::transport