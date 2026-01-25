/**
 * @file zmq_transport.cpp
 * @brief ZeroMQ transport implementation
 */

#include "sensorstreamkit/transport/zmq_transport.hpp"

namespace sensorstreamkit::transport {

ZmqTransport::ZmqTransport() : context_(std::make_unique<zmq::context_t>(1)) {
    frontend_socket_ = std::make_unique<zmq::socket_t>(*context_, ZMQ_XSUB);
    backend_socket_ = std::make_unique<zmq::socket_t>(*context_, ZMQ_XPUB);
}
    


ZmqTransport::ZmqTransport(ZmqTransport&& other) noexcept
    : context_(std::move(other.context_)),
      frontend_socket_(std::move(other.frontend_socket_)),
      backend_socket_(std::move(other.backend_socket_)) {
    // Moved-from object's unique_ptr will be null, which is valid
}

ZmqTransport& ZmqTransport::operator=(ZmqTransport&& other) noexcept {
    if (this != &other) {
        // Reset sockets first (closes them via unique_ptr), then close context
        frontend_socket_.reset();
        backend_socket_.reset();
        if (context_) {
            context_->close();
            context_.reset();
        }

        // Transfer ownership
        context_ = std::move(other.context_);
        frontend_socket_ = std::move(other.frontend_socket_);
        backend_socket_ = std::move(other.backend_socket_);
    }
    return *this;
}

ZmqTransport::~ZmqTransport() {
    // Reset sockets first (closes them via unique_ptr), then close context
    frontend_socket_.reset();
    backend_socket_.reset();
    if (context_) {
        context_->close();
        context_.reset();
    }
}

void ZmqTransport::run_broker(const std::string& frontend_endpoint, const std::string& backend_endpoint) {
    try {
        // 1. Inbound (From Publishers): XSUB
        frontend_socket_->bind(frontend_endpoint);

        // 2. Outbound (To Subscribers): XPUB
        backend_socket_->bind(backend_endpoint);

        // 3. Start the proxy (blocks indefinitely)
        zmq::proxy(*frontend_socket_, *backend_socket_);
    } catch (const zmq::error_t& e) {
        if (e.num() != ETERM) {
            throw;
        }
    }
}

} // namespace sensorstreamkit::transport