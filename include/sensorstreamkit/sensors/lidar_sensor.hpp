#pragma once

#include "sensorstreamkit/sensors/sensor_base.hpp"
#include "sensorstreamkit/transport/zmq_publisher.hpp"
#include "sensorstreamkit/core/message.hpp"
#include <chrono>

namespace sensorstreamkit::sensors {

/**
 * @brief Simulated LiDAR sensor
 */
class LidarSensor : public ISensor {
public:
    LidarSensor(std::string id, 
                transport::ZmqPublisher& publisher, 
                std::chrono::milliseconds interval = std::chrono::milliseconds(100))
        : id_(std::move(id))
        , publisher_(publisher)
        , interval_(interval)
        , periodic_pub_(publisher_, "lidar", [this]() { return generate_data(); }, interval_) {}

    [[nodiscard]] std::string id() const override { return id_; }
    [[nodiscard]] std::string type() const override { return "lidar"; }

    void start() override {
        periodic_pub_.start();
    }

    void stop() override {
        periodic_pub_.stop();
    }

    [[nodiscard]] bool is_active() const override {
        return periodic_pub_.is_running();
    }

private:
    core::LidarScanData generate_data() {
        return core::LidarScanData{
            .sensor_id_ = id_,
            .timestamp_ns_ = core::Timestamp::now().nanoseconds(),
            .num_points = 100000,
            .scan_duration_ms = static_cast<float>(interval_.count())
        };
    }

    std::string id_;
    transport::ZmqPublisher& publisher_;
    std::chrono::milliseconds interval_;
    transport::PeriodicPublisher<core::LidarScanData> periodic_pub_;
};

} // namespace sensorstreamkit::sensors
