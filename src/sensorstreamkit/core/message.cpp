/**
 * @file message.cpp
 * @brief Message serialization implementation
 */

#include "sensorstreamkit/core/message.hpp"
#include <cstring>

namespace sensorstreamkit::core {
ConstPayload MessageHeader::serialize(std::vector<uint8_t>& buffer) const {
    // Resize buffer to exact size needed - eliminates sizing errors
    buffer.resize(serialized_size);

    std::memcpy(buffer.data(), &timestamp_ns, sizeof(timestamp_ns));
    std::memcpy(buffer.data() + 8, &sequence_number, sizeof(sequence_number));
    std::memcpy(buffer.data() + 12, &message_type, sizeof(message_type));
    std::memcpy(buffer.data() + 14, &reserved, sizeof(reserved));

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
    size_t required_size = sizeof(uint32_t) + sensor_id_.size() +
                           sizeof(timestamp_ns_) +
                           sizeof(width) + sizeof(height) +
                           sizeof(uint32_t) + encoding.size();

    // Resize buffer to exact size needed - eliminates sizing errors
    buffer.resize(required_size);

    size_t offset = 0;

    uint32_t id_len = static_cast<uint32_t>(sensor_id_.size());
    std::memcpy(buffer.data() + offset, &id_len, sizeof(id_len));
    offset += sizeof(id_len);
    std::memcpy(buffer.data() + offset, sensor_id_.data(), sensor_id_.size());
    offset += sensor_id_.size();

    std::memcpy(buffer.data() + offset, &timestamp_ns_, sizeof(timestamp_ns_));
    offset += sizeof(timestamp_ns_);
    std::memcpy(buffer.data() + offset, &frame_id, sizeof(frame_id));
    offset += sizeof(frame_id);
    std::memcpy(buffer.data() + offset, &width, sizeof(width));
    offset += sizeof(width);
    std::memcpy(buffer.data() + offset, &height, sizeof(height));
    offset += sizeof(height);

    uint32_t enc_len = static_cast<uint32_t>(encoding.size());
    std::memcpy(buffer.data() + offset, &enc_len, sizeof(enc_len));
    offset += sizeof(enc_len);
    std::memcpy(buffer.data() + offset, encoding.data(), encoding.size());
    offset += encoding.size();

    return ConstPayload(buffer.data(), buffer.size());

}

}   // namespace sensorstreamkit::core