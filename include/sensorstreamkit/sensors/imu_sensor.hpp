#pragma once

#include "sensorstreamkit/sensors/sensor_base.hpp"
#include "sensorstreamkit/transport/zmq_publisher.hpp"
#include "sensorstreamkit/core/message.hpp"
#include <chrono>
#include <random>

namespace sensorstreamkit::sensors {

/**
 * @brief Simulated IMU sensor
 */
class ImuSensor : public ISensor {
public:
    ImuSensor(std::string id, 
              transport::ZmqPublisher& publisher, 
              std::chrono::milliseconds interval = std::chrono::milliseconds(10))
        : id_(std::move(id))
        , publisher_(publisher)
        , interval_(interval)
        , periodic_pub_(publisher_, "imu", [this]() { return generate_data(); }, interval_)
        , gen_(rd_())
        , dist_(-0.1f, 0.1f) {}

    [[nodiscard]] std::string id() const override { return id_; }
    [[nodiscard]] std::string type() const override { return "imu"; }

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
    core::ImuData generate_data() {
        return core::ImuData{
            .sensor_id_ = id_,
            .timestamp_ns_ = core::Timestamp::now().nanoseconds(),
            .accel_x = dist_(gen_),
            .accel_y = dist_(gen_),
            .accel_z = 9.81f + dist_(gen_),
            .gyro_x = dist_(gen_),
            .gyro_y = dist_(gen_),
            .gyro_z = dist_(gen_)
        };
    }

    std::string id_;
    transport::ZmqPublisher& publisher_;
    std::chrono::milliseconds interval_;
    transport::PeriodicPublisher<core::ImuData> periodic_pub_;

    // Random noise generator
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_real_distribution<float> dist_;
};

} // namespace sensorstreamkit::sensors
