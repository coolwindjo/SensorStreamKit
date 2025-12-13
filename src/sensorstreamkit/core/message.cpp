/**
 * @file message.cpp
 * @brief Message serialization implementation
 */

#include "sensorstreamkit/core/message.hpp"
#include <cstring>

namespace sensorstreamkit::core {

ConstPayload MessageHeader::serialize(std::vector<uint8_t>& buffer) const {
    // Resize buffer to exact size needed - eliminates sizing errors
    const size_t offset = buffer.size();
    buffer.resize(offset + serialized_size);

    std::memcpy(buffer.data() + offset, &timestamp_ns, sizeof(timestamp_ns));
    std::memcpy(buffer.data() + offset + 8, &sequence_number, sizeof(sequence_number));
    std::memcpy(buffer.data() + offset + 12, &message_type, sizeof(message_type));
    std::memcpy(buffer.data() + offset + 14, &reserved, sizeof(reserved));

    return ConstPayload(buffer.data(), buffer.size());
}

std::optional<MessageHeader> MessageHeader::deserialize(ConstPayload data) {
    if (data.size() < serialized_size) {
        return std::nullopt;
    }

    MessageHeader header;
    std::memcpy(&header.timestamp_ns, data.data(), sizeof(header.timestamp_ns));
    std::memcpy(&header.sequence_number, data.data() + 8, sizeof(header.sequence_number));
    std::memcpy(&header.message_type, data.data() + 12, sizeof(header.message_type));
    std::memcpy(&header.reserved, data.data() + 14, sizeof(header.reserved));

    return header;
}

ConstPayload CameraFrameData::serialize(std::vector<uint8_t>& buffer) const {
    const size_t required_size = sizeof(uint32_t) + sensor_id_.size() +
                                 sizeof(timestamp_ns_) +
                                 sizeof(frame_id) +
                                 sizeof(width) + sizeof(height) +
                                 sizeof(uint32_t) + encoding.size();

    // Resize buffer to exact size needed - eliminates sizing errors
    size_t offset = buffer.size();
    buffer.resize(offset + required_size);

    // Serialize sensor_id length + data
    uint32_t id_len = static_cast<uint32_t>(sensor_id_.size());
    std::memcpy(buffer.data() + offset, &id_len, sizeof(id_len));
    offset += sizeof(id_len);
    std::memcpy(buffer.data() + offset, sensor_id_.data(), sensor_id_.size());
    offset += sensor_id_.size();

    // Serialize fixed fields
    std::memcpy(buffer.data() + offset, &timestamp_ns_, sizeof(timestamp_ns_));
    offset += sizeof(timestamp_ns_);
    std::memcpy(buffer.data() + offset, &frame_id, sizeof(frame_id));
    offset += sizeof(frame_id);
    std::memcpy(buffer.data() + offset, &width, sizeof(width));
    offset += sizeof(width);
    std::memcpy(buffer.data() + offset, &height, sizeof(height));
    offset += sizeof(height);

    // Serialize encoding
    uint32_t enc_len = static_cast<uint32_t>(encoding.size());
    std::memcpy(buffer.data() + offset, &enc_len, sizeof(enc_len));
    offset += sizeof(enc_len);
    std::memcpy(buffer.data() + offset, encoding.data(), encoding.size());

    return ConstPayload(buffer.data(), buffer.size());
}

std::optional<CameraFrameData> CameraFrameData::deserialize(ConstPayload data) {
    if (data.size() < sizeof(uint32_t)) return std::nullopt;

    CameraFrameData result;
    size_t offset = 0;

    // Deserialize sensor_id
    uint32_t id_len;
    std::memcpy(&id_len, data.data() + offset, sizeof(id_len));
    offset += sizeof(id_len);
    if (data.size() < offset + id_len) return std::nullopt;

    result.sensor_id_.assign(reinterpret_cast<const char*>(data.data() + offset), id_len);
    offset += id_len;

    // Deserialize fixed fields
    if (data.size() < offset + sizeof(result.timestamp_ns_) + sizeof(result.frame_id) +
                      sizeof(result.width) + sizeof(result.height)) return std::nullopt;

    std::memcpy(&result.timestamp_ns_, data.data() + offset, sizeof(result.timestamp_ns_));
    offset += sizeof(result.timestamp_ns_);
    std::memcpy(&result.frame_id, data.data() + offset, sizeof(result.frame_id));
    offset += sizeof(result.frame_id);
    std::memcpy(&result.width, data.data() + offset, sizeof(result.width));
    offset += sizeof(result.width);
    std::memcpy(&result.height, data.data() + offset, sizeof(result.height));
    offset += sizeof(result.height);

    // Deserialize encoding
    if (data.size() < offset + sizeof(uint32_t)) return std::nullopt;
    uint32_t enc_len;
    std::memcpy(&enc_len, data.data() + offset, sizeof(enc_len));
    offset += sizeof(enc_len);
    if (data.size() < offset + enc_len) return std::nullopt;
    result.encoding.assign(reinterpret_cast<const char*>(data.data() + offset), enc_len);

    return result;
}

ConstPayload LidarScanData::serialize(std::vector<uint8_t>& buffer) const {
    const size_t required_size = sizeof(uint32_t) + sensor_id_.size() +
                                 sizeof(timestamp_ns_) +
                                 sizeof(num_points) +
                                 sizeof(scan_duration_ms)

    // Resize buffer to exact size needed - eliminates sizing errors
    size_t offset = buffer.size();
    buffer.resize(offset + required_size);

    // Serialize sensor_id length + data
    uint32_t id_len = static_cast<uint32_t>(sensor_id_.size());
    std::memcpy(buffer.data() + offset, &id_len, sizeof(id_len));
    offset += sizeof(id_len);
    std::memcpy(buffer.data() + offset, sensor_id_.data(), sensor_id_.size());
    offset += sensor_id_.size();

    // Serialize fixed fields
    std::memcpy(buffer.data() + offset, &timestamp_ns_, sizeof(timestamp_ns_));
    offset += sizeof(timestamp_ns_);
    std::memcpy(buffer.data() + offset, &num_points, sizeof(num_points));
    offset += sizeof(num_points);
    std::memcpy(buffer.data() + offset, &scan_duration_ms, sizeof(scan_duration_ms));

    return ConstPayload(buffer.data(), buffer.size());
}

std::optional<LidarScanData> LidarScanData::deserialize(ConstPayload data) {
    if (data.size() < sizeof(uint32_t)) return std::nullopt;

    LidarScanData result;
    size_t offset = 0;

    // Deserialize sensor_id
    uint32_t id_len;
    std::memcpy(&id_len, data.data() + offset, sizeof(id_len));
    offset += sizeof(id_len);
    if (data.size() < offset + id_len) return std::nullopt;

    result.sensor_id_.assign(reinterpret_cast<const char*>(data.data() + offset), id_len);
    offset += id_len;

    // Deserialize fixed fields
    constexpr size_t fixed_size = sizeof(result.timestamp_ns_) +
                                  sizeof(result.num_points) +
                                  sizeof(result.scan_duration_ms);
    if (data.size() < offset + fixed_size) return std::nullopt;

    std::memcpy(&result.timestamp_ns_, data.data() + offset, sizeof(result.timestamp_ns_));
    offset += sizeof(result.timestamp_ns_);
    std::memcpy(&result.num_points, data.data() + offset, sizeof(result.num_points));
    offset += sizeof(result.num_points);
    std::memcpy(&result.scan_duration_ms, data.data() + offset, sizeof(result.scan_duration_ms));

    return result;
}

ConstPayload ImuData::serialize(std::vector<uint8_t>& buffer) const {
    const size_t required_size = sizeof(uint32_t) + sensor_id_.size() +
                                 sizeof(timestamp_ns_) +
                                 sizeof(accel_x) + sizeof(accel_y) + sizeof(accel_z) +
                                 sizeof(gyro_x) + sizeof(gyro_y) + sizeof(gyro_z);

    // Resize buffer to exact size needed - eliminates sizing errors
    size_t offset = buffer.size();
    buffer.resize(offset + required_size);

    // Serialize sensor_id length + data
    uint32_t id_len = static_cast<uint32_t>(sensor_id_.size());
    std::memcpy(buffer.data() + offset, &id_len, sizeof(id_len));
    offset += sizeof(id_len);
    std::memcpy(buffer.data() + offset, sensor_id_.data(), sensor_id_.size());
    offset += sensor_id_.size();

    // Serialize fixed fields
    std::memcpy(buffer.data() + offset, &timestamp_ns_, sizeof(timestamp_ns_));
    offset += sizeof(timestamp_ns_);
    std::memcpy(buffer.data() + offset, &accel_x, sizeof(accel_x));
    offset += sizeof(accel_x);
    std::memcpy(buffer.data() + offset, &accel_y, sizeof(accel_y));
    offset += sizeof(accel_y);
    std::memcpy(buffer.data() + offset, &accel_z, sizeof(accel_z));
    offset += sizeof(accel_z);
    std::memcpy(buffer.data() + offset, &gyro_x, sizeof(gyro_x));
    offset += sizeof(gyro_x);
    std::memcpy(buffer.data() + offset, &gyro_y, sizeof(gyro_y));
    offset += sizeof(gyro_y);
    std::memcpy(buffer.data() + offset, &gyro_z, sizeof(gyro_z));

    return ConstPayload(buffer.data(), buffer.size());
}

std::optional<ImuData> ImuData::deserialize(ConstPayload data) {
    if (data.size() < sizeof(uint32_t)) return std::nullopt;

    ImuData result;
    size_t offset = 0;

    // Deserialize sensor_id
    uint32_t id_len;
    std::memcpy(&id_len, data.data() + offset, sizeof(id_len));
    offset += sizeof(id_len);
    if (data.size() < offset + id_len) return std::nullopt;

    result.sensor_id_.assign(reinterpret_cast<const char*>(data.data() + offset), id_len);
    offset += id_len;

    // Deserialize fixed fields
    constexpr size_t fixed_size = sizeof(result.timestamp_ns_) +
                                  sizeof(result.accel_x) + sizeof(result.accel_y) + sizeof(result.accel_z) +
                                  sizeof(result.gyro_x) + sizeof(result.gyro_y) + sizeof(result.gyro_z);
    if (data.size() < offset + fixed_size) return std::nullopt;

    std::memcpy(&result.timestamp_ns_, data.data() + offset, sizeof(result.timestamp_ns_));
    offset += sizeof(result.timestamp_ns_);
    std::memcpy(&result.accel_x, data.data() + offset, sizeof(result.accel_x));
    offset += sizeof(result.accel_x);
    std::memcpy(&result.accel_y, data.data() + offset, sizeof(result.accel_y));
    offset += sizeof(result.accel_y);
    std::memcpy(&result.accel_z, data.data() + offset, sizeof(result.accel_z));
    offset += sizeof(result.accel_z);
    std::memcpy(&result.gyro_x, data.data() + offset, sizeof(result.gyro_x));
    offset += sizeof(result.gyro_x);
    std::memcpy(&result.gyro_y, data.data() + offset, sizeof(result.gyro_y));
    offset += sizeof(result.gyro_y);
    std::memcpy(&result.gyro_z, data.data() + offset, sizeof(result.gyro_z));

    return result;
}

}   // namespace sensorstreamkit::core