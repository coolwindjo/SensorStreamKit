#pragma once

#include "sensorstreamkit/sensors/sensor_base.hpp"
#include "sensorstreamkit/transport/zmq_publisher.hpp"
#include "sensorstreamkit/core/message.hpp"
#include <chrono>

namespace sensorstreamkit::sensors {

/**
 * @brief Simulated Camera sensor
 */
class CameraSensor : public ISensor {
public:
    CameraSensor(std::string id, 
                 transport::ZmqPublisher& publisher, 
                 std::chrono::milliseconds interval = std::chrono::milliseconds(33))
        : id_(std::move(id))
        , publisher_(publisher)
        , interval_(interval)
        , periodic_pub_(publisher_, "camera", [this]() { return generate_data(); }, interval_) {}

    [[nodiscard]] std::string id() const override { return id_; }
    [[nodiscard]] std::string type() const override { return "camera"; }

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
    core::CameraFrameData generate_data() {
        return core::CameraFrameData{
            .sensor_id_ = id_,
            .timestamp_ns_ = core::Timestamp::now().nanoseconds(),
            .frame_id = frame_id_++,
            .width = 1920,
            .height = 1080,
            .encoding = "RGB8"
        };
    }

    std::string id_;
    transport::ZmqPublisher& publisher_;
    std::chrono::milliseconds interval_;
    uint32_t frame_id_{0};
    transport::PeriodicPublisher<core::CameraFrameData> periodic_pub_;
};

} // namespace sensorstreamkit::sensors
