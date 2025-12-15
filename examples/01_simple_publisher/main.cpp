/**
 * @file main.cpp
 * @brief Simple publisher example for SensorStreamKit
 *
 * This example demonstrates how to create a simple publisher that sends
 * camera frames, LiDAR scans, and IMU data using the SensorStreamKit library.
 *
 * @author Jo, SeungHyeon (Jo,SH)
 * @date 2025
*/

#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <csignal>
#include <atomic>

#include "sensorstreamkit/core/message.hpp"
#include "sensorstreamkit/transport/zmq_publisher.hpp"

using namespace sensorstreamkit::core;
using namespace sensorstreamkit::transport;
using namespace std::chrono_literals;

// Global flag for graceful shutdown
std::atomic<bool> g_running{true};

void signal_handler(int) {
    std::cout << "\nShutdown requested..." << std::endl;
    g_running = false;
}

/**
 * @brief Simulated camera data generator
 */
class SimulatedCamera {
public:
    explicit SimulatedCamera(std::string_view id) : sensor_id_(id) {}

    CameraFrameData generate() {
        return CameraFrameData{
            .sensor_id_ = sensor_id_,
            .timestamp_ns_ = Timestamp::now().nanoseconds(),
            .frame_id = frame_counter_++,
            .width = 1920,
            .height = 1080,
            .encoding = "RGB8"
        };
    }
private:
    std::string sensor_id_;
    uint32_t frame_counter_{0};
};

int main() {
    // Setup signal handler for graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "=== SensorStreamKit Simple Publisher ===" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;

    // Create ZeroMQ publisher
    PublisherConfig config {
        .endpoint = "tcp://*:5555",
        .high_water_mark = 1000,
        .send_timeout_ms = 1000,
        .conflate = false
    };

    // Create and bind publisher
    ZmqPublisher publisher(config);
    if (!publisher.bind()) {
        std::cerr << "Failed to bind publisher to endpoint: " << config.endpoint << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Publisher bound to " << config.endpoint << std::endl;

    // Create simulated camera
    SimulatedCamera camera("Camera_01");

    // Publishing interval
    constexpr auto publish_interval = 33ms;  // ~30 Hz

    // Publish loop
    while (g_running) {
        // Generate camera frame data
        CameraFrameData frame = camera.generate();

        // Create message
        Message<CameraFrameData> msg(std::move(frame));

        // Publish message
        if (publisher.publish("camera_frames", msg)) {
            if (frame.frame_id % 30 == 0)  // Log every 30 frames / second
                std::cout << "Published frame ID: " << msg.payload().frame_id
                          << " Timestamp (ns): " << msg.payload().timestamp_ns()
                          << " Sensor ID: " << msg.payload().sensor_id()
                          << " | Total messages sent: " << publisher.messages_sent() << std::endl;
        } else {
            std::cerr << "Failed to publish frame ID: " << msg.payload().frame_id << std::endl;
        }

        // Sleep for a while before next frame
        std::this_thread::sleep_for(publish_interval);
    }

    std::cout << "Total messages sent: " << publisher.messages_sent() << std::endl;
    std::cout << "Publisher shutting down." << std::endl;

    return EXIT_SUCCESS;
}