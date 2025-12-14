#pragma once

/**
 * @file message.hpp
 * @brief Core message types with C++20 concepts for type-safe sensor data
 * @author Jo, SeungHyeon (Jo,SH)
 * @date 2025
 */

#include <chrono>
#include <concepts>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace sensorstreamkit::core {

// ============================================================================
// Type definitions for Payloads
// ============================================================================

using ConstPayload = std::span<const uint8_t>;

// ===========================================================================
// Concepts for Type Safety
// ============================================================================

/**
 * @brief Concept for types that can be serialized to binary format
 */
template <typename T>
concept Serializable = requires(const T& t, std::vector<uint8_t>& buffer) {
    { t.serialize(buffer) } -> std::same_as<void>;
    { T::deserialize(ConstPayload{}) } -> std::same_as<std::optional<T>>;
};

/**
 * @brief Concept for sensor data types
 */
template <typename T>
concept SensorDataType = requires(const T& t) {
    { t.timestamp_ns() } -> std::convertible_to<uint64_t>;
    { t.sensor_id() } -> std::convertible_to<std::string_view>;
} && Serializable<T> && std::is_class_v<T> && std::semiregular<T>;

/**
 * @brief Payload structures must satisfy SensorDataType concept
 */
template <typename T>
concept PayloadType = SensorDataType<T>;

// ============================================================================
// Timestamp Utilities
// ============================================================================

/**
 * @brief High-precision timestamp using std::chrono
 */
class Timestamp {
public:
    using clock_type = std::chrono::steady_clock;
    using time_point = clock_type::time_point;
    // using duration = clock_type::duration;

    Timestamp() noexcept : tp_(clock_type::now()) {}
    explicit Timestamp(uint64_t ns) noexcept
        : tp_(time_point(std::chrono::nanoseconds(ns))) {}

    [[nodiscard]] uint64_t nanoseconds() const noexcept {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            tp_.time_since_epoch()).count();
    }

    [[nodiscard]] double seconds() const noexcept {
        return std::chrono::duration<double>(tp_.time_since_epoch()).count();
    }

    [[nodiscard]] static Timestamp now() noexcept { return Timestamp{}; }

    auto operator<=>(const Timestamp&) const = default;

private:
    time_point tp_;
};

// ============================================================================
// Base Message Header
// ============================================================================

/**
 * @brief Common header for all sensor messages
 */
struct MessageHeader {
    uint64_t timestamp_ns{0};
    uint32_t sequence_number{0};
    uint16_t message_type{0};
    uint16_t reserved{0};   // For future use

    static constexpr size_t serialized_size = sizeof(timestamp_ns) +
                                              sizeof(sequence_number) +
                                              sizeof(message_type) +
                                              sizeof(reserved);

    void serialize(std::vector<uint8_t>& buffer) const;
    static std::optional<MessageHeader> deserialize(ConstPayload data);
};

// ============================================================================
// Generic Message Wrapper with C++20 Concepts
// ============================================================================

/**
 * @brief Type-safe message wrapper using C++20 concepts
 */
template <SensorDataType T>
class Message {
public:
    Message() = default;

    explicit Message(T payload)
        : header_{Timestamp::now().nanoseconds(), next_sequence(), 0, 0}
        , payload_(std::move(payload)) {}

    [[nodiscard]] const MessageHeader& header() const noexcept { return header_; }
    [[nodiscard]] const T& payload() const noexcept { return payload_; }
    [[nodiscard]] T& payload() noexcept { return payload_; }

    void serialize(std::vector<uint8_t>& buffer) const {
        header_.serialize(buffer);
        payload_.serialize(buffer);
    }

    static std::optional<Message<T>> deserialize(ConstPayload data) {
        if (data.size() < MessageHeader::serialized_size) {
            return std::nullopt; }

        auto header = MessageHeader::deserialize(data.subspan(0, MessageHeader::serialized_size));
        if (!header) return std::nullopt;

        auto payload = T::deserialize(data.subspan(MessageHeader::serialized_size));
        if (!payload) return std::nullopt;

        Message<T> msg;
        msg.header_ = *header;
        msg.payload_ = std::move(*payload);
        return msg;
    }

private:
    MessageHeader header_;
    T payload_;

    static uint32_t next_sequence() {
        static uint32_t seq = 0;
        return seq++;
    }
};

// ============================================================================
// Sensor Data Structures
// ============================================================================

/**
 * @brief Camera frame metadata (not actual pixels)
 */
struct CameraFrameData {
    std::string sensor_id_;
    uint64_t timestamp_ns_{0};
    uint32_t frame_id{0};
    uint32_t width{0};
    uint32_t height{0};
    std::string encoding;  // e.g., "RGB8", "MONO8", "BAYER_RGGB8"

    [[nodiscard]] uint64_t timestamp_ns() const noexcept { return timestamp_ns_; }
    [[nodiscard]] std::string_view sensor_id() const noexcept { return sensor_id_; }

    void serialize(std::vector<uint8_t>& buffer) const;
    static std::optional<CameraFrameData> deserialize(ConstPayload data);
};

/**
 * @brief Lidar point cloud metadata (not actual points)
 */
struct LidarScanData {
    std::string sensor_id_;
    uint64_t timestamp_ns_{0};
    uint32_t num_points{0};
    float scan_duration_ms{0.0f};

    [[nodiscard]] uint64_t timestamp_ns() const noexcept { return timestamp_ns_; }
    [[nodiscard]] std::string_view sensor_id() const noexcept { return sensor_id_; }

    void serialize(std::vector<uint8_t>& buffer) const;
    static std::optional<LidarScanData> deserialize(ConstPayload data);
};

/**
 * @brief IMU (Inertial Measurement Unit) data
 */
struct ImuData {
    std::string sensor_id_;
    uint64_t timestamp_ns_{0};

    // Linear acceleration (m/s²)
    float accel_x{0.0f};
    float accel_y{0.0f};
    float accel_z{0.0f};
    // Angular velocity (rad/s)
    float gyro_x{0.0f};
    float gyro_y{0.0f};
    float gyro_z{0.0f};

    [[nodiscard]] uint64_t timestamp_ns() const noexcept { return timestamp_ns_; }
    [[nodiscard]] std::string_view sensor_id() const noexcept { return sensor_id_; }

    void serialize(std::vector<uint8_t>& buffer) const;
    static std::optional<ImuData> deserialize(ConstPayload data);
};

// Verify concepts are satisfied
static_assert(Serializable<MessageHeader>);
static_assert(SensorDataType<CameraFrameData>);
static_assert(SensorDataType<LidarScanData>);
static_assert(SensorDataType<ImuData>);

}  // namespace sensorstreamkit::core