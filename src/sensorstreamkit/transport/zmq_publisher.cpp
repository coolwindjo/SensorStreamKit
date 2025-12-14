/**
 * @file zmq_publisher.cpp
 * @brief ZeroMQ publisher implementation
 */

#include "sensorstreamkit/transport/zmq_publisher.hpp"
#include <stdexcept>

namespace sensorstreamkit::transport {

ZmqPublisher::ZmqPublisher(const PublisherConfig& config)
    : config_(config)
    , context_(std::make_unique<zmq::context_t>(1))
    , socket_(std::make_unique<zmq::socket_t>(*context_, zmq::socket_type::pub)) {

    socket_->set(zmq::sockopt::sndhwm, config_.high_water_mark);
    socket_->set(zmq::sockopt::sndtimeo, config_.send_timeout_ms);

    if (config_.conflate) {
        socket_->set(zmq::sockopt::conflate, 1);
    }
}

ZmqPublisher::~ZmqPublisher() {
    if (socket_) {
        socket_->close();
    }
    if (context_) {
        context_->close();
    }
}

ZmqPublisher::ZmqPublisher(ZmqPublisher&& other) noexcept
    : config_(std::move(other.config_))
    , context_(std::move(other.context_))
    , socket_(std::move(other.socket_))
    , messages_sent_(other.messages_sent_.load())
    , bound_(other.bound_.load()) {
}

ZmqPublisher& ZmqPublisher::operator=(ZmqPublisher&& other) noexcept {
    if (this != &other) {
        config_ = std::move(other.config_);
        context_ = std::move(other.context_);
        socket_ = std::move(other.socket_);
        messages_sent_ = other.messages_sent_.load();
        bound_ = other.bound_.load();
    }
    return *this;
}

bool ZmqPublisher::bind() {
    try {
        socket_->bind(config_.endpoint);
        bound_ = true;
        return true;
    } catch (const zmq::error_t& e) {
        bound_ = false;
        return false;
    }
}

bool ZmqPublisher::publish_raw(std::string_view topic, std::span<const uint8_t> data) {
    if (!bound_) {
        return false;  // Not bound
    }

    try {
        zmq::message_t topic_msg(topic.data(), topic.size());
        zmq::message_t data_msg(data.data(), data.size());

        socket_->send(topic_msg, zmq::send_flags::sndmore);
        socket_->send(data_msg, zmq::send_flags::none);

        messages_sent_.fetch_add(1, std::memory_order_relaxed);

        return true;
    } catch (const zmq::error_t& e) {
        return false;
    }
}

} // namespace sensorstreamkit::transport