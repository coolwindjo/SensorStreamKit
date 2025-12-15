/**
 * @file test_message_class.cpp
 * @brief Comprehensive unit tests for Message<T> template class
 * @author Jo, SeungHyeon (Jo,SH)
 * @date 2025
 */

#include <gtest/gtest.h>
#include "sensorstreamkit/core/message.hpp"
#include <thread>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace sensorstreamkit::core;

// ============================================================================
// Test Fixture for Message<CameraFrameData>
// ============================================================================

class MessageCameraTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create sample camera frame data
        sample_camera_data_ = CameraFrameData{
            .sensor_id_ = "camera_front",
            .timestamp_ns_ = 1234567890123456789ULL,
            .frame_id = 42,
            .width = 1920,
            .height = 1080,
            .encoding = "RGB8"
        };
    }

    CameraFrameData sample_camera_data_;
};

// ============================================================================
// Test Fixture for Message<LidarScanData>
// ============================================================================

class MessageLidarTest : public ::testing::Test {
protected:
    void SetUp() override {
        sample_lidar_data_ = LidarScanData{
            .sensor_id_ = "lidar_roof",
            .timestamp_ns_ = 5555555555555555555ULL,
            .num_points = 100000,
            .scan_duration_ms = 100.5f
        };
    }

    LidarScanData sample_lidar_data_;
};

// ============================================================================
// Test Fixture for Message<ImuData>
// ============================================================================

class MessageImuTest : public ::testing::Test {
protected:
    void SetUp() override {
        sample_imu_data_ = ImuData{
            .sensor_id_ = "imu_main",
            .timestamp_ns_ = 9876543210987654321ULL,
            .accel_x = 1.5f,
            .accel_y = -2.3f,
            .accel_z = 9.8f,
            .gyro_x = 0.1f,
            .gyro_y = -0.05f,
            .gyro_z = 0.02f
        };
    }

    ImuData sample_imu_data_;
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(MessageCameraTest, DefaultConstructor) {
    Message<CameraFrameData> msg;

    // Header should be default-initialized
    EXPECT_EQ(msg.header().timestamp_ns, 0);
    EXPECT_EQ(msg.header().sequence_number, 0);
    EXPECT_EQ(msg.header().message_type, 0);
    EXPECT_EQ(msg.header().reserved, 0);

    // Payload should be default-constructed
    EXPECT_EQ(msg.payload().sensor_id_, "");
    EXPECT_EQ(msg.payload().timestamp_ns_, 0);
    EXPECT_EQ(msg.payload().frame_id, 0);
    EXPECT_EQ(msg.payload().width, 0);
    EXPECT_EQ(msg.payload().height, 0);
    EXPECT_EQ(msg.payload().encoding, "");
}

TEST_F(MessageCameraTest, PayloadConstructor) {
    Message<CameraFrameData> msg(sample_camera_data_);

    // Header should have timestamp and sequence number set
    EXPECT_GT(msg.header().timestamp_ns, 0);
    EXPECT_GE(msg.header().sequence_number, 0);

    // Payload should match the input
    EXPECT_EQ(msg.payload().sensor_id_, "camera_front");
    EXPECT_EQ(msg.payload().timestamp_ns_, 1234567890123456789ULL);
    EXPECT_EQ(msg.payload().frame_id, 42);
    EXPECT_EQ(msg.payload().width, 1920);
    EXPECT_EQ(msg.payload().height, 1080);
    EXPECT_EQ(msg.payload().encoding, "RGB8");
}

TEST_F(MessageImuTest, PayloadConstructorImu) {
    Message<ImuData> msg(sample_imu_data_);

    EXPECT_GT(msg.header().timestamp_ns, 0);
    EXPECT_EQ(msg.payload().sensor_id_, "imu_main");
    EXPECT_FLOAT_EQ(msg.payload().accel_x, 1.5f);
    EXPECT_FLOAT_EQ(msg.payload().gyro_z, 0.02f);
}

TEST_F(MessageLidarTest, PayloadConstructorLidar) {
    Message<LidarScanData> msg(sample_lidar_data_);

    EXPECT_GT(msg.header().timestamp_ns, 0);
    EXPECT_EQ(msg.payload().sensor_id_, "lidar_roof");
    EXPECT_EQ(msg.payload().num_points, 100000);
    EXPECT_FLOAT_EQ(msg.payload().scan_duration_ms, 100.5f);
}

// ============================================================================
// Accessor Tests - header() const
// ============================================================================

TEST_F(MessageCameraTest, HeaderAccessorConst) {
    const Message<CameraFrameData> msg(sample_camera_data_);

    const MessageHeader& header = msg.header();
    EXPECT_GT(header.timestamp_ns, 0);
    EXPECT_GE(header.sequence_number, 0);

    // Verify it returns a reference (not a copy)
    EXPECT_EQ(&msg.header(), &header);
}

// ============================================================================
// Accessor Tests - payload() const
// ============================================================================

TEST_F(MessageCameraTest, PayloadAccessorConst) {
    const Message<CameraFrameData> msg(sample_camera_data_);

    const CameraFrameData& payload = msg.payload();
    EXPECT_EQ(payload.sensor_id_, "camera_front");
    EXPECT_EQ(payload.width, 1920);

    // Verify it returns a reference (not a copy)
    EXPECT_EQ(&msg.payload(), &payload);
}

// ============================================================================
// Accessor Tests - payload() non-const
// ============================================================================

TEST_F(MessageCameraTest, PayloadAccessorNonConst) {
    Message<CameraFrameData> msg(sample_camera_data_);

    // Modify through non-const accessor
    msg.payload().width = 3840;
    msg.payload().height = 2160;
    msg.payload().encoding = "RGB16";

    // Verify modifications persisted
    EXPECT_EQ(msg.payload().width, 3840);
    EXPECT_EQ(msg.payload().height, 2160);
    EXPECT_EQ(msg.payload().encoding, "RGB16");
}

TEST_F(MessageImuTest, PayloadMutability) {
    Message<ImuData> msg(sample_imu_data_);

    msg.payload().accel_x = 99.9f;
    msg.payload().gyro_y = -88.8f;

    EXPECT_FLOAT_EQ(msg.payload().accel_x, 99.9f);
    EXPECT_FLOAT_EQ(msg.payload().gyro_y, -88.8f);
}

// ============================================================================
// Sequence Number Tests
// ============================================================================

TEST_F(MessageCameraTest, SequenceNumberIncremental) {
    Message<CameraFrameData> msg1(sample_camera_data_);
    Message<CameraFrameData> msg2(sample_camera_data_);
    Message<CameraFrameData> msg3(sample_camera_data_);

    // Sequence numbers should be incrementing
    EXPECT_LT(msg1.header().sequence_number, msg2.header().sequence_number);
    EXPECT_LT(msg2.header().sequence_number, msg3.header().sequence_number);

    // Should differ by at least 1
    EXPECT_GE(msg2.header().sequence_number - msg1.header().sequence_number, 1);
    EXPECT_GE(msg3.header().sequence_number - msg2.header().sequence_number, 1);
}

TEST_F(MessageImuTest, SequenceNumberUnique) {
    std::vector<uint32_t> sequence_numbers;

    for (int i = 0; i < 100; ++i) {
        Message<ImuData> msg(sample_imu_data_);
        sequence_numbers.push_back(msg.header().sequence_number);
    }

    // All sequence numbers should be unique
    std::sort(sequence_numbers.begin(), sequence_numbers.end());
    auto it = std::adjacent_find(sequence_numbers.begin(), sequence_numbers.end());
    EXPECT_EQ(it, sequence_numbers.end()) << "Found duplicate sequence number: " << *it;
}

TEST(MessageSequenceTest, ThreadSafeSequenceGeneration) {
    constexpr int num_threads = 10;
    constexpr int msgs_per_thread = 100;
    std::vector<std::thread> threads;
    std::vector<std::vector<uint32_t>> all_sequences(num_threads);

    ImuData dummy_data{
        .sensor_id_ = "test",
        .timestamp_ns_ = 0
    };

    // Create messages from multiple threads
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < msgs_per_thread; ++i) {
                Message<ImuData> msg(dummy_data);
                all_sequences[t].push_back(msg.header().sequence_number);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Collect all sequence numbers
    std::vector<uint32_t> all_nums;
    for (const auto& vec : all_sequences) {
        all_nums.insert(all_nums.end(), vec.begin(), vec.end());
    }

    // Verify all are unique (no race conditions)
    std::sort(all_nums.begin(), all_nums.end());
    auto it = std::adjacent_find(all_nums.begin(), all_nums.end());
    EXPECT_EQ(it, all_nums.end()) << "Thread-safety violated: duplicate sequence number " << *it;
}

// ============================================================================
// Serialization Tests - serialize()
// ============================================================================

TEST_F(MessageCameraTest, SerializeBasic) {
    Message<CameraFrameData> msg(sample_camera_data_);
    std::vector<uint8_t> buffer;

    msg.serialize(buffer);

    // Buffer should not be empty
    EXPECT_GT(buffer.size(), 0);

    // Should at least contain header
    EXPECT_GE(buffer.size(), MessageHeader::serialized_size);
}

TEST_F(MessageCameraTest, SerializeToEmptyBuffer) {
    Message<CameraFrameData> msg(sample_camera_data_);
    std::vector<uint8_t> buffer;  // Empty buffer

    msg.serialize(buffer);

    EXPECT_GT(buffer.size(), 0);
}

TEST_F(MessageCameraTest, SerializeAppendsToBuffer) {
    Message<CameraFrameData> msg(sample_camera_data_);
    std::vector<uint8_t> buffer;

    // Pre-populate buffer with some data
    buffer.push_back(0xAA);
    buffer.push_back(0xBB);
    buffer.push_back(0xCC);
    size_t original_size = buffer.size();

    msg.serialize(buffer);

    // Buffer should have grown
    EXPECT_GT(buffer.size(), original_size);

    // Original data should still be at the beginning
    EXPECT_EQ(buffer[0], 0xAA);
    EXPECT_EQ(buffer[1], 0xBB);
    EXPECT_EQ(buffer[2], 0xCC);
}

TEST_F(MessageImuTest, SerializeImuData) {
    Message<ImuData> msg(sample_imu_data_);
    std::vector<uint8_t> buffer;

    msg.serialize(buffer);

    EXPECT_GT(buffer.size(), MessageHeader::serialized_size);
}

TEST_F(MessageLidarTest, SerializeLidarData) {
    Message<LidarScanData> msg(sample_lidar_data_);
    std::vector<uint8_t> buffer;

    msg.serialize(buffer);

    EXPECT_GT(buffer.size(), MessageHeader::serialized_size);
}

// ============================================================================
// Deserialization Tests - deserialize()
// ============================================================================

TEST_F(MessageCameraTest, DeserializeEmptyData) {
    std::vector<uint8_t> empty_buffer;

    auto result = Message<CameraFrameData>::deserialize(empty_buffer);

    EXPECT_FALSE(result.has_value());
}

TEST_F(MessageCameraTest, DeserializeTooSmall) {
    std::vector<uint8_t> tiny_buffer(MessageHeader::serialized_size - 1);

    auto result = Message<CameraFrameData>::deserialize(tiny_buffer);

    EXPECT_FALSE(result.has_value());
}

TEST_F(MessageCameraTest, DeserializeHeaderOnly) {
    std::vector<uint8_t> buffer(MessageHeader::serialized_size);

    auto result = Message<CameraFrameData>::deserialize(buffer);

    // Should fail because payload is missing
    EXPECT_FALSE(result.has_value());
}

TEST_F(MessageCameraTest, SerializeDeserializeRoundTrip) {
    Message<CameraFrameData> original(sample_camera_data_);
    std::vector<uint8_t> buffer;

    // Serialize
    original.serialize(buffer);

    // Deserialize
    auto result = Message<CameraFrameData>::deserialize(buffer);

    ASSERT_TRUE(result.has_value());

    // Verify header matches
    EXPECT_EQ(result->header().timestamp_ns, original.header().timestamp_ns);
    EXPECT_EQ(result->header().sequence_number, original.header().sequence_number);
    EXPECT_EQ(result->header().message_type, original.header().message_type);
    EXPECT_EQ(result->header().reserved, original.header().reserved);

    // Verify payload matches
    EXPECT_EQ(result->payload().sensor_id_, original.payload().sensor_id_);
    EXPECT_EQ(result->payload().timestamp_ns_, original.payload().timestamp_ns_);
    EXPECT_EQ(result->payload().frame_id, original.payload().frame_id);
    EXPECT_EQ(result->payload().width, original.payload().width);
    EXPECT_EQ(result->payload().height, original.payload().height);
    EXPECT_EQ(result->payload().encoding, original.payload().encoding);
}

TEST_F(MessageImuTest, SerializeDeserializeRoundTripImu) {
    Message<ImuData> original(sample_imu_data_);
    std::vector<uint8_t> buffer;

    original.serialize(buffer);
    auto result = Message<ImuData>::deserialize(buffer);

    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result->payload().sensor_id_, original.payload().sensor_id_);
    EXPECT_FLOAT_EQ(result->payload().accel_x, original.payload().accel_x);
    EXPECT_FLOAT_EQ(result->payload().accel_y, original.payload().accel_y);
    EXPECT_FLOAT_EQ(result->payload().accel_z, original.payload().accel_z);
    EXPECT_FLOAT_EQ(result->payload().gyro_x, original.payload().gyro_x);
    EXPECT_FLOAT_EQ(result->payload().gyro_y, original.payload().gyro_y);
    EXPECT_FLOAT_EQ(result->payload().gyro_z, original.payload().gyro_z);
}

TEST_F(MessageLidarTest, SerializeDeserializeRoundTripLidar) {
    Message<LidarScanData> original(sample_lidar_data_);
    std::vector<uint8_t> buffer;

    original.serialize(buffer);
    auto result = Message<LidarScanData>::deserialize(buffer);

    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result->payload().sensor_id_, original.payload().sensor_id_);
    EXPECT_EQ(result->payload().timestamp_ns_, original.payload().timestamp_ns_);
    EXPECT_EQ(result->payload().num_points, original.payload().num_points);
    EXPECT_FLOAT_EQ(result->payload().scan_duration_ms, original.payload().scan_duration_ms);
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST_F(MessageCameraTest, EmptyStringsInPayload) {
    CameraFrameData empty_strings{
        .sensor_id_ = "",
        .timestamp_ns_ = 123,
        .frame_id = 1,
        .width = 640,
        .height = 480,
        .encoding = ""
    };

    Message<CameraFrameData> msg(empty_strings);
    std::vector<uint8_t> buffer;

    msg.serialize(buffer);
    auto result = Message<CameraFrameData>::deserialize(buffer);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->payload().sensor_id_, "");
    EXPECT_EQ(result->payload().encoding, "");
    EXPECT_EQ(result->payload().width, 640);
}

TEST_F(MessageCameraTest, VeryLongStringsInPayload) {
    std::string long_id(10000, 'A');
    std::string long_encoding(5000, 'B');

    CameraFrameData long_strings{
        .sensor_id_ = long_id,
        .timestamp_ns_ = 999,
        .frame_id = 5,
        .width = 7680,
        .height = 4320,
        .encoding = long_encoding
    };

    Message<CameraFrameData> msg(long_strings);
    std::vector<uint8_t> buffer;

    msg.serialize(buffer);
    auto result = Message<CameraFrameData>::deserialize(buffer);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->payload().sensor_id_, long_id);
    EXPECT_EQ(result->payload().encoding, long_encoding);
}

TEST_F(MessageImuTest, ExtremeFloatValues) {
    ImuData extreme{
        .sensor_id_ = "extreme",
        .timestamp_ns_ = 0,
        .accel_x = std::numeric_limits<float>::max(),
        .accel_y = std::numeric_limits<float>::lowest(),
        .accel_z = 0.0f,
        .gyro_x = std::numeric_limits<float>::epsilon(),
        .gyro_y = -std::numeric_limits<float>::epsilon(),
        .gyro_z = std::numeric_limits<float>::infinity()
    };

    Message<ImuData> msg(extreme);
    std::vector<uint8_t> buffer;

    msg.serialize(buffer);
    auto result = Message<ImuData>::deserialize(buffer);

    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(result->payload().accel_x, std::numeric_limits<float>::max());
    EXPECT_FLOAT_EQ(result->payload().accel_y, std::numeric_limits<float>::lowest());
    EXPECT_EQ(result->payload().gyro_z, std::numeric_limits<float>::infinity());
}

TEST_F(MessageCameraTest, MultipleMessagesInOneBuffer) {
    Message<CameraFrameData> msg1(sample_camera_data_);

    CameraFrameData data2 = sample_camera_data_;
    data2.frame_id = 999;
    Message<CameraFrameData> msg2(data2);

    std::vector<uint8_t> buffer;

    // Serialize both messages into same buffer
    msg1.serialize(buffer);
    size_t first_msg_size = buffer.size();
    msg2.serialize(buffer);

    // Deserialize first message
    auto result1 = Message<CameraFrameData>::deserialize(
        ConstPayload(buffer.data(), first_msg_size)
    );

    // Deserialize second message
    auto result2 = Message<CameraFrameData>::deserialize(
        ConstPayload(buffer.data() + first_msg_size, buffer.size() - first_msg_size)
    );

    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());

    EXPECT_EQ(result1->payload().frame_id, 42);
    EXPECT_EQ(result2->payload().frame_id, 999);
}

// ============================================================================
// Copy and Move Semantics Tests
// ============================================================================

TEST_F(MessageCameraTest, CopyConstructor) {
    Message<CameraFrameData> original(sample_camera_data_);
    Message<CameraFrameData> copy(original);

    // Should have same content
    EXPECT_EQ(copy.header().timestamp_ns, original.header().timestamp_ns);
    EXPECT_EQ(copy.header().sequence_number, original.header().sequence_number);
    EXPECT_EQ(copy.payload().sensor_id_, original.payload().sensor_id_);
    EXPECT_EQ(copy.payload().width, original.payload().width);
}

TEST_F(MessageCameraTest, CopyAssignment) {
    Message<CameraFrameData> original(sample_camera_data_);
    Message<CameraFrameData> copy;

    copy = original;

    EXPECT_EQ(copy.header().timestamp_ns, original.header().timestamp_ns);
    EXPECT_EQ(copy.payload().sensor_id_, original.payload().sensor_id_);
}

TEST_F(MessageCameraTest, MoveConstructor) {
    Message<CameraFrameData> original(sample_camera_data_);
    uint64_t original_timestamp = original.header().timestamp_ns;
    std::string original_id = original.payload().sensor_id_;

    Message<CameraFrameData> moved(std::move(original));

    EXPECT_EQ(moved.header().timestamp_ns, original_timestamp);
    EXPECT_EQ(moved.payload().sensor_id_, original_id);
}

TEST_F(MessageCameraTest, MoveAssignment) {
    Message<CameraFrameData> original(sample_camera_data_);
    uint64_t original_timestamp = original.header().timestamp_ns;
    std::string original_id = original.payload().sensor_id_;

    Message<CameraFrameData> moved;
    moved = std::move(original);

    EXPECT_EQ(moved.header().timestamp_ns, original_timestamp);
    EXPECT_EQ(moved.payload().sensor_id_, original_id);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
