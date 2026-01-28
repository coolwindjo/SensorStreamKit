#pragma once

#include "sensorstreamkit/sensors/sensor_base.hpp"
#include <map>
#include <memory>
#include <vector>
#include <string>

namespace sensorstreamkit::sensors {

/**
 * @brief Registry and manager for sensor instances
 */
class SensorManager {
public:
    SensorManager() = default;
    ~SensorManager() = default;

    /**
     * @brief Register a sensor
     */
    void add_sensor(std::shared_ptr<ISensor> sensor) {
        if (sensor) {
            sensors_[sensor->id()] = std::move(sensor);
        }
    }

    /**
     * @brief Find a sensor by ID
     */
    [[nodiscard]] std::shared_ptr<ISensor> get_sensor(const std::string& id) const {
        auto it = sensors_.find(id);
        if (it != sensors_.end()) {
            return it->second;
        }
        return nullptr;
    }

    /**
     * @brief Get all registered sensors
     */
    [[nodiscard]] std::vector<std::shared_ptr<ISensor>> get_all_sensors() const {
        std::vector<std::shared_ptr<ISensor>> result;
        result.reserve(sensors_.size());
        for (const auto& [id, sensor] : sensors_) {
            result.push_back(sensor);
        }
        return result;
    }

    /**
     * @brief Start all sensors
     */
    void start_all() {
        for (auto& [id, sensor] : sensors_) {
            sensor->start();
        }
    }

    /**
     * @brief Stop all sensors
     */
    void stop_all() {
        for (auto& [id, sensor] : sensors_) {
            sensor->stop();
        }
    }

private:
    std::map<std::string, std::shared_ptr<ISensor>> sensors_;
};

} // namespace sensorstreamkit::sensors
