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
}

ZmqTransport& ZmqTransport::operator=(ZmqTransport&& other) noexcept {
    if (this != &other) {
        context_ = std::move(other.context_);
        frontend_socket_ = std::move(other.frontend_socket_);
        backend_socket_ = std::move(other.backend_socket_);
    }
    return *this;
}

ZmqTransport::~ZmqTransport() {
    if (context_) {
        context_->close();
        context_.reset();
        frontend_socket_.reset();
        backend_socket_.reset();
    }
}

void ZmqTransport::run_broker(const std::string& frontend_endpoint, const std::string& backend_endpoint) {
    // 1. Inbound (From Publishers): XSUB
    frontend_socket_->bind(frontend_endpoint);

    // 2. Outbound (To Subscribers): XPUB
    backend_socket_->bind(backend_endpoint);

    // 3. Start the proxy (blocks indefinitely)
    zmq::proxy(*frontend_socket_, *backend_socket_);
}

} // namespace sensorstreamkit::transport