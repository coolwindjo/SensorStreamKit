/**
 * @file zmq_subscriber.cpp
 * @brief ZeroMQ subscriber implementation
 */

#include "sensorstreamkit/transport/zmq_subscriber.hpp"
#include <stdexcept>
#include <chrono>

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
    // Reset moved-from object to valid state
    other.messages_received_.store(0, std::memory_order_relaxed);
    other.connected_.store(false, std::memory_order_relaxed);
}

ZmqSubscriber& ZmqSubscriber::operator=(ZmqSubscriber&& other) noexcept {
    if (this != &other) {
        // Close existing resources if any
        if (socket_) {
            socket_->close();
        }
        if (context_) {
            context_->close();
        }

        // Transfer ownership
        config_ = std::move(other.config_);
        context_ = std::move(other.context_);
        socket_ = std::move(other.socket_);
        messages_received_ = other.messages_received_.load();
        connected_ = other.connected_.load();

        // Reset moved-from object to valid state
        other.messages_received_.store(0, std::memory_order_relaxed);
        other.connected_.store(false, std::memory_order_relaxed);
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

std::optional<std::vector<uint8_t>> ZmqSubscriber::receive_raw(std::stop_token stoken) {
    if (!connected_) {
        return std::nullopt;
    }

    using namespace std::chrono;
    auto start_time = steady_clock::now();
    auto timeout = milliseconds(config_.receive_timeout_ms);
    bool infinite_timeout = (config_.receive_timeout_ms < 0);

    while (!stoken.stop_requested()) {
        zmq::pollitem_t items[] = { { *socket_, 0, ZMQ_POLLIN, 0 } };
        
        milliseconds poll_duration = milliseconds(100);

        if (!infinite_timeout) {
            auto elapsed = duration_cast<milliseconds>(steady_clock::now() - start_time);
            if (elapsed >= timeout) {
                return std::nullopt; // Timeout
            }
            auto remaining = timeout - elapsed;
            if (remaining < poll_duration) {
                poll_duration = remaining;
            }
        }

        int rc = zmq::poll(items, 1, poll_duration);

        if (rc < 0) {
            return std::nullopt; // Error in poll
        }

        if (rc > 0 && (items[0].revents & ZMQ_POLLIN)) {
            try {
                // Receive topic (first part of multipart message)
                zmq::message_t topic_msg;
                auto result = socket_->recv(topic_msg, zmq::recv_flags::none);
                if (!result) {
                    return std::nullopt;  // Timeout or error
                }

                bool has_more = socket_->get(zmq::sockopt::rcvmore);
                if (!has_more) {
                    // Received a message with only one part, which is not expected.
                    return std::nullopt;
                }

                // Receive data (second part of multipart message)
                zmq::message_t data_msg;
                result = socket_->recv(data_msg, zmq::recv_flags::none);
                if (!result) {
                    return std::nullopt;  // Timeout or error
                }

                // Consume unexpected extra parts
                while (socket_->get(zmq::sockopt::rcvmore)) {
                    zmq::message_t extra_msg;
                    (void)socket_->recv(extra_msg, zmq::recv_flags::none);
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
    }
    return std::nullopt; // Stop requested
}

} // namespace sensorstreamkit::transport