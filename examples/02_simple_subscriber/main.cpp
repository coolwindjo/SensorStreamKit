/**
 * @file main.cpp
 * @brief Simple subscriber example for SensorStreamKit
 *
 * This example demonstrates how to create a simple subscriber that receives
 * camera frames using the SensorStreamKit library.
 *
 * @author Jo, SeungHyeon (Jo,SH)
 * @date 2025
*/

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>

#include "sensorstreamkit/transport/zmq_subscriber.hpp"

using namespace sensorstreamkit::transport;
using namespace std::chrono_literals;

int main() {
    SubscriberConfig config;
    // Connect to the broker's backend (XPUB)
    config.endpoint = "tcp://localhost:5556";
    config.receive_timeout_ms = 100; // Non-blocking-ish
    
    std::cout << "Subscriber initializing..." << std::endl;
    ZmqSubscriber subscriber(config);
    
    std::cout << "Connecting to " << config.endpoint << "..." << std::endl;
    if (!subscriber.connect()) {
        std::cerr << "Failed to connect to broker!" << std::endl;
        return 1;
    }
    
    // Subscribe to "camera_frames" topic
    if (subscriber.subscribe("camera_frames")) {
        std::cout << "Subscribed to 'camera_frames'" << std::endl;
    } else {
        std::cerr << "Failed to subscribe!" << std::endl;
        return 1;
    }

    // Receiving loop

    size_t message_received = 0;
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
        auto result = subscriber.receive<CameraFrameData>();

        if (result) {
            const auto& frame = result->payload();

            message_received++;

            // Log every 30 messages
            if (message_received % 30 == 0) {
                auto elapsed = std::chrono::steady_clock::now() - start_time;
                auto elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
                double fps = message_received / (elapsed_sec > 0 ? elapsed_sec : 1);

                std::cout << "Received frame ID: " << frame.frame_id
                          << " Timestamp (ns): " << frame.timestamp_ns()
                          << " Sensor ID: " << frame.sensor_id()
                          << " | Total messages: " << message_received << std::endl;
            }
        }
    }

    std::cout << "\nTotal messages received: " << message_received << std::endl;
    std::cout << "Subscriber stopped." << std::endl;

    return 0;
}
