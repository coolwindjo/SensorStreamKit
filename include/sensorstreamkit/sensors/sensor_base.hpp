#pragma once

#include <string>
#include <string_view>

namespace sensorstreamkit::sensors {

/**
 * @brief Interface for all sensor types
 */
class ISensor {
public:
    virtual ~ISensor() = default;

    /**
     * @brief Get unique sensor identifier
     */
    [[nodiscard]] virtual std::string id() const = 0;

    /**
     * @brief Get sensor type (e.g., "camera", "lidar", "imu")
     */
    [[nodiscard]] virtual std::string type() const = 0;

    /**
     * @brief Start sensor data generation and transmission
     */
    virtual void start() = 0;

    /**
     * @brief Stop sensor
     */
    virtual void stop() = 0;

    /**
     * @brief Check if sensor is currently active
     */
    [[nodiscard]] virtual bool is_active() const = 0;
};

} // namespace sensorstreamkit::sensors
