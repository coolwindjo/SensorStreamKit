#include "sensorstreamkit/api/rest_server.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace sensorstreamkit::api;

int main() {
    RestServer server(8080);

    // Health check endpoint
    server.get("/health", [](const Request&) {
        std::cout << "[API] Health check requested" << std::endl;
        return Response::json({{"status", "ok"}, {"version", "1.0.0"}});
    });

    // Get sensor list (mock)
    server.get("/sensors", [](const Request&) {
        std::cout << "[API] Sensor list requested" << std::endl;
        return Response::json({
            {"sensors", {
                {{"id", "camera_front"}, {"type", "camera"}, {"status", "streaming"}},
                {{"id", "lidar_top"}, {"type", "lidar"}, {"status", "idle"}},
                {{"id", "imu_main"}, {"type", "imu"}, {"status", "streaming"}}
            }}
        });
    });

    // Start a sensor (mock)
    server.post("/sensors/start", [](const Request& req) {
        std::string sensor_id = req.get_param("id");
        if (sensor_id.empty()) {
            return Response::json({{"status", "error"}, {"message", "Missing sensor id"}});
        }
        std::cout << "[API] Starting sensor: " << sensor_id << std::endl;
        return Response::json({{"status", "ok"}, {"sensor_id", sensor_id}});
    });

    // Run server in a separate thread so we can stop it if needed
    // In this simple example, we'll just run it blocking
    std::cout << "REST API Server Example" << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET  http://localhost:8080/health" << std::endl;
    std::cout << "  GET  http://localhost:8080/sensors" << std::endl;
    std::cout << "  POST http://localhost:8080/sensors/start?id=camera_front" << std::endl;
    
    server.run();

    return 0;
}
