#include "sensorstreamkit/api/rest_server.hpp"
#include "sensorstreamkit/sensors/sensor_manager.hpp"
#include "sensorstreamkit/sensors/camera_sensor.hpp"
#include "sensorstreamkit/sensors/lidar_sensor.hpp"
#include "sensorstreamkit/sensors/imu_sensor.hpp"
#include "sensorstreamkit/transport/zmq_publisher.hpp"
#include <iostream>
#include <csignal>

using namespace sensorstreamkit;

// Global flag for graceful shutdown
std::atomic<bool> running{true};
void signal_handler(int) { running = false; }

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // 1. Initialize Transport
    transport::PublisherConfig pub_config{.endpoint = "tcp://*:5555"};
    transport::ZmqPublisher publisher(pub_config);
    if (!publisher.bind()) {
        std::cerr << "Failed to bind publisher" << std::endl;
        return 1;
    }

    // 2. Initialize Sensors
    auto sensor_manager = std::make_shared<sensors::SensorManager>();
    
    sensor_manager->add_sensor(std::make_shared<sensors::CameraSensor>(
        "camera_front", publisher, std::chrono::milliseconds(33)));
    
    sensor_manager->add_sensor(std::make_shared<sensors::LidarSensor>(
        "lidar_top", publisher, std::chrono::milliseconds(100)));
    
    sensor_manager->add_sensor(std::make_shared<sensors::ImuSensor>(
        "imu_main", publisher, std::chrono::milliseconds(10)));

    // 3. Initialize REST API
    api::RestServer api_server(8080);

    // GET /health
    api_server.get("/health", [](const api::Request&) {
        return api::Response::json({{"status", "ok"}});
    });

    // GET /sensors
    api_server.get("/sensors", [sensor_manager](const api::Request&) {
        nlohmann::json sensor_list = nlohmann::json::array();
        for (const auto& sensor : sensor_manager->get_all_sensors()) {
            sensor_list.push_back({
                {"id", sensor->id()},
                {"type", sensor->type()},
                {"active", sensor->is_active()}
            });
        }
        return api::Response::json({{"sensors", sensor_list}});
    });

    // POST /sensors/start?id=xxx
    api_server.post("/sensors/start", [sensor_manager](const api::Request& req) {
        std::string id = req.get_param("id");
        auto sensor = sensor_manager->get_sensor(id);
        if (sensor) {
            sensor->start();
            return api::Response::json({{"status", "ok"}, {"message", "Sensor started"}});
        }
        return api::Response::json({{"status", "error"}, {"message", "Sensor not found"}});
    });

    // POST /sensors/stop?id=xxx
    api_server.post("/sensors/stop", [sensor_manager](const api::Request& req) {
        std::string id = req.get_param("id");
        auto sensor = sensor_manager->get_sensor(id);
        if (sensor) {
            sensor->stop();
            return api::Response::json({{"status", "ok"}, {"message", "Sensor stopped"}});
        }
        return api::Response::json({{"status", "error"}, {"message", "Sensor not found"}});
    });

    // 4. Run System
    std::cout << "SensorStreamKit Integrated System Started" << std::endl;
    std::cout << "- ZMQ Publisher: tcp://*:5555" << std::endl;
    std::cout << "- REST API: http://localhost:8080" << std::endl;

    // Start API server in a separate thread
    std::thread api_thread([&api_server]() {
        api_server.run();
    });

    // Main loop
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Shutting down..." << std::endl;
    api_server.stop();
    if (api_thread.joinable()) api_thread.join();
    sensor_manager->stop_all();

    return 0;
}
