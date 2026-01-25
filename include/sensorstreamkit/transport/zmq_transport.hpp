#pragma once

/**
 * @file zmq_transport.hpp
 * @brief ZeroMQ transport components for SensorStreamKit
 * @author Jo, SeungHyeon (Jo,SH)
 */

#include <zmq.hpp>

#include "sensorstreamkit/transport/zmq_publisher.hpp"
#include "sensorstreamkit/transport/zmq_subscriber.hpp"

namespace sensorstreamkit::transport {

class ZmqTransport {
public:
    ZmqTransport();
    ~ZmqTransport();

    // Non-copyable, movable
    ZmqTransport(const ZmqTransport&) = delete;
    ZmqTransport& operator=(const ZmqTransport&) = delete;
    ZmqTransport(ZmqTransport&&) noexcept;
    ZmqTransport& operator=(ZmqTransport&&) noexcept;

    /**
     * @brief Run a ZeroMQ proxy broker (blocking)
     * @param frontend_endpoint Address for publishers to connect (XSUB) e.g., "tcp://0.0.0.0:5555"
     * @param backend_endpoint Address for subscribers to connect (XPUB) e.g., "tcp://0.0.0.0:5556"
     */
    void run_broker(const std::string& frontend_endpoint, const std::string& backend_endpoint);

    /**
     * @brief Shutdown the broker context to unblock run_broker().
     */
    void shutdown() {
        if (context_) {
            context_->shutdown();
        }
    }

private:
    std::unique_ptr<zmq::context_t> context_;
    std::unique_ptr<zmq::socket_t> frontend_socket_;
    std::unique_ptr<zmq::socket_t> backend_socket_;

};

}  // namespace sensorstreamkit::transport