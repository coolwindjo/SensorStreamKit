/**
 * @file zmq_publisher.cpp
 * @brief ZeroMQ publisher implementation
 */

#include "sensorstreamkit/transport/zmq_publisher.hpp"
#include <stdexcept>
#include <chrono>

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
    , messages_sent_(other.messages_sent_.load(std::memory_order_relaxed))
    , bound_(other.bound_.load(std::memory_order_relaxed)) {
    // Reset moved-from object to valid state
    other.messages_sent_.store(0, std::memory_order_relaxed);
    other.bound_.store(false, std::memory_order_relaxed);
}

ZmqPublisher& ZmqPublisher::operator=(ZmqPublisher&& other) noexcept {
    if (this != &other) {
        ZmqPublisher temp(std::move(other));
        swap(temp);
    }
    return *this;
}

void ZmqPublisher::swap(ZmqPublisher& other) noexcept {
    using std::swap;
    swap(config_, other.config_);
    swap(context_, other.context_);
    swap(socket_, other.socket_);

    // Swap atomics (not natively swappable)
    uint64_t ms = messages_sent_.load(std::memory_order_relaxed);
    messages_sent_.store(other.messages_sent_.load(std::memory_order_relaxed), std::memory_order_relaxed);
    other.messages_sent_.store(ms, std::memory_order_relaxed);

    bool b = bound_.load(std::memory_order_relaxed);
    bound_.store(other.bound_.load(std::memory_order_relaxed), std::memory_order_relaxed);
    other.bound_.store(b, std::memory_order_relaxed);
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

bool ZmqPublisher::connect() {
    try {
        socket_->connect(config_.endpoint);
        bound_ = true;
        return true;
    } catch (const zmq::error_t& e) {
        bound_ = false;
        return false;
    }
}

bool ZmqPublisher::publish_raw(std::string_view topic, std::span<const uint8_t> data, std::stop_token stoken) {
    if (!bound_) {
        return false;  // Not bound
    }

    using namespace std::chrono;
    auto start_time = steady_clock::now();
    auto timeout = milliseconds(config_.send_timeout_ms);
    bool infinite_timeout = (config_.send_timeout_ms < 0);

    while (!stoken.stop_requested()) {
        zmq::pollitem_t items[] = { { *socket_, 0, ZMQ_POLLOUT, 0 } };
        
        milliseconds poll_duration = milliseconds(100);

        if (!infinite_timeout) {
            auto elapsed = duration_cast<milliseconds>(steady_clock::now() - start_time);
            if (elapsed >= timeout) {
                return false; // Timeout
            }
            auto remaining = timeout - elapsed;
            if (remaining < poll_duration) {
                poll_duration = remaining;
            }
        }

        int rc = zmq::poll(items, 1, poll_duration);

        if (rc < 0) {
            return false; // Error in poll
        }

        if (rc > 0 && (items[0].revents & ZMQ_POLLOUT)) {
            try {
                zmq::message_t topic_msg(topic.data(), topic.size());
                socket_->send(topic_msg, zmq::send_flags::sndmore);
                zmq::message_t data_msg(data.data(), data.size());
                socket_->send(data_msg, zmq::send_flags::none);
                messages_sent_.fetch_add(1, std::memory_order_relaxed);
                return true;
            } catch (const zmq::error_t&) {
                return false;
            }
        }
    }
    return false; // Stop requested
}

} // namespace sensorstreamkit::transport