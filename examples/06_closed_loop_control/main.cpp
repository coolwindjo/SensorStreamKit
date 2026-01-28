#include "sensorstreamkit/api/rest_server.hpp"
#include "sensorstreamkit/sensors/sensor_manager.hpp"
#include "sensorstreamkit/sensors/camera_sensor.hpp"
#include "sensorstreamkit/transport/zmq_publisher.hpp"
#include "sensorstreamkit/transport/zmq_subscriber.hpp"
#include <iostream>
#include <iomanip>

using namespace sensorstreamkit;

/**
 * This example demonstrates a closed-loop system where:
 * 1. A SensorManager manages a camera.
 * 2. A REST API allows controlling that camera.
 * 3. A background thread acts as a 'client' that receives the data.
 */

int main() {
    // 1. Setup Transport (Publisher)
    transport::PublisherConfig pub_config{.endpoint = "tcp://127.0.0.1:5555"};
    transport::ZmqPublisher publisher(pub_config);
    publisher.bind();

    // 2. Setup Sensor
    auto manager = std::make_shared<sensors::SensorManager>();
    auto camera = std::make_shared<sensors::CameraSensor>("webcam", publisher, std::chrono::milliseconds(100));
    manager->add_sensor(camera);

    // 3. Setup Subscriber (to verify data flows)
    std::atomic<int> frames_received{0};
    std::jthread sub_thread([&]() {
        transport::ZmqSubscriber subscriber;
        subscriber.connect("tcp://127.0.0.1:5555");
        subscriber.subscribe("camera");

        while (true) {
            auto msg = subscriber.receive<core::CameraFrameData>(std::chrono::milliseconds(500));
            if (msg) {
                frames_received++;
            }
        }
    });

    // 4. Setup REST API
    api::RestServer api(8080);
    api.get("/status", [&](const api::Request&) {
        return api::Response::json({
            {"sensor_id", camera->id()},
            {"is_active", camera->is_active()},
            {"frames_sent", frames_received.load()} // Simplified for demo
        });
    });

    api.post("/control", [&](const api::Request& req) {
        std::string cmd = req.get_param("cmd");
        if (cmd == "start") {
            camera->start();
            return api::Response::json({{"result", "started"}});
        } else if (cmd == "stop") {
            camera->stop();
            return api::Response::json({{"result", "stopped"}});
        }
        return api::Response::json({{"result", "invalid command"}});
    });

    std::cout << "--- Closed-Loop Control Example ---" << std::endl;
    std::cout << "1. Check status: curl http://localhost:8080/status" << std::endl;
    std::cout << "2. Start camera: curl -X POST http://localhost:8080/control?cmd=start" << std::endl;
    std::cout << "3. Stop camera:  curl -X POST http://localhost:8080/control?cmd=stop" << std::endl;
    
    api.run();

    return 0;
}
