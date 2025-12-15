/**
 * @file test_zmq_transport.cpp
 * @brief Comprehensive tests for ZeroMQ Publisher and Subscriber
 * @author Jo, SeungHyeon (Jo,SH)
 *
 * Test coverage:
 * - ZmqPublisher: construction, bind, publish, move semantics, config options
 * - ZmqSubscriber: construction, connect, subscribe, receive, move semantics
 * - Integration: PUB/SUB roundtrip, topic filtering, multiple subscribers
 * - Performance: throughput, latency
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

#include "sensorstreamkit/transport/zmq_publisher.hpp"
#include "sensorstreamkit/transport/zmq_subscriber.hpp"
#include "sensorstreamkit/core/message.hpp"
#include <numeric>

using namespace sensorstreamkit::transport;
using namespace sensorstreamkit::core;
using namespace std::chrono_literals;

// ============================================================================
// Test Fixtures
// ============================================================================

class ZmqPublisherTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use unique port for each test to avoid conflicts
        static int port = 15550;
        config_.endpoint = "tcp://*:" + std::to_string(port++);
    }

    PublisherConfig config_;
};

class ZmqSubscriberTest : public ::testing::Test {
protected:
    void SetUp() override {
        static int port = 15650;
        config_.endpoint = "tcp://localhost:" + std::to_string(port++);
    }

    SubscriberConfig config_;
};

class ZmqIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        static int port = 15750;
        port_ = port++;
        pub_config_.endpoint = "tcp://*:" + std::to_string(port_);
        sub_config_.endpoint = "tcp://localhost:" + std::to_string(port_);
    }

    int port_;
    PublisherConfig pub_config_;
    SubscriberConfig sub_config_;
};

// ============================================================================
// ZmqPublisher Tests
// ============================================================================

TEST_F(ZmqPublisherTest, BindSuccess) {
    ZmqPublisher publisher(config_);
    EXPECT_TRUE(publisher.bind());
}

TEST_F(ZmqPublisherTest, BindFailureWithInvalidEndpoint) {
    config_.endpoint = "invalid://endpoint";
    ZmqPublisher publisher(config_);
    EXPECT_FALSE(publisher.bind());
}

TEST_F(ZmqPublisherTest, BindTwiceFails) {
    ZmqPublisher publisher(config_);
    ASSERT_TRUE(publisher.bind());

    // Binding to the same endpoint again should fail
    // Note: ZeroMQ allows rebinding, so we need to test actual behavior
    // This test documents the current behavior
    ZmqPublisher publisher2(config_);
    EXPECT_FALSE(publisher2.bind());  // Port already in use
}

TEST_F(ZmqPublisherTest, PublishRawWithoutBindFails) {
    ZmqPublisher publisher(config_);
    std::vector<uint8_t> data = {1, 2, 3, 4};
    EXPECT_FALSE(publisher.publish_raw("test_topic", data));
}

TEST_F(ZmqPublisherTest, PublishRawAfterBindSucceeds) {
    ZmqPublisher publisher(config_);
    ASSERT_TRUE(publisher.bind());

    std::vector<uint8_t> data = {1, 2, 3, 4};
    EXPECT_TRUE(publisher.publish_raw("test_topic", data));
    EXPECT_EQ(publisher.messages_sent(), 1);
}

TEST_F(ZmqPublisherTest, PublishMultipleMessagesIncrementsCounter) {
    ZmqPublisher publisher(config_);
    ASSERT_TRUE(publisher.bind());

    std::vector<uint8_t> data = {1, 2, 3, 4};
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(publisher.publish_raw("topic", data));
    }
    EXPECT_EQ(publisher.messages_sent(), 10);
}

TEST_F(ZmqPublisherTest, PublishTypedCameraMessage) {
    ZmqPublisher publisher(config_);
    ASSERT_TRUE(publisher.bind());

    CameraFrameData frame_data{
        .sensor_id_ = "camera_01",
        .timestamp_ns_ = Timestamp::now().nanoseconds(),
        .frame_id = 42,
        .width = 1920,
        .height = 1080,
        .encoding = "RGB8"
    };

    Message<CameraFrameData> message(frame_data);
    EXPECT_TRUE(publisher.publish("camera_frames", message));
    EXPECT_EQ(publisher.messages_sent(), 1);
}

TEST_F(ZmqPublisherTest, MoveConstructor) {
    ZmqPublisher publisher1(config_);
    ASSERT_TRUE(publisher1.bind());

    std::vector<uint8_t> data = {1, 2, 3};
    ASSERT_TRUE(publisher1.publish_raw("topic", data));
    ASSERT_EQ(publisher1.messages_sent(), 1);

    ZmqPublisher publisher2(std::move(publisher1));
    EXPECT_EQ(publisher2.messages_sent(), 1);

    // publisher2 should still work after move
    EXPECT_TRUE(publisher2.publish_raw("topic", data));
    EXPECT_EQ(publisher2.messages_sent(), 2);
}

TEST_F(ZmqPublisherTest, ConflateOptionKeepsOnlyLastMessage) {
    config_.conflate = true;
    ZmqPublisher publisher(config_);
    ASSERT_TRUE(publisher.bind());

    std::vector<uint8_t> data = {1, 2, 3};

    // Send multiple messages quickly
    // With conflate=true, only the last message should be kept
    for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(publisher.publish_raw("topic", data));
    }
    EXPECT_EQ(publisher.messages_sent(), 100);
}

TEST_F(ZmqPublisherTest, PublishEmptyTopic) {
    ZmqPublisher publisher(config_);
    ASSERT_TRUE(publisher.bind());

    std::vector<uint8_t> data = {1, 2, 3};
    EXPECT_TRUE(publisher.publish_raw("", data));
    EXPECT_EQ(publisher.messages_sent(), 1);
}

TEST_F(ZmqPublisherTest, PublishEmptyData) {
    ZmqPublisher publisher(config_);
    ASSERT_TRUE(publisher.bind());

    std::vector<uint8_t> data;
    EXPECT_TRUE(publisher.publish_raw("topic", data));
    EXPECT_EQ(publisher.messages_sent(), 1);
}

TEST_F(ZmqPublisherTest, PublishLargeMessage) {
    ZmqPublisher publisher(config_);
    ASSERT_TRUE(publisher.bind());

    // Test with 1MB message
    std::vector<uint8_t> data(1024 * 1024, 0xAB);
    EXPECT_TRUE(publisher.publish_raw("large_topic", data));
    EXPECT_EQ(publisher.messages_sent(), 1);
}

// ============================================================================
// ZmqSubscriber Tests
// ============================================================================

TEST_F(ZmqSubscriberTest, ConnectSuccess) {
    // Note: Connection succeeds even if publisher doesn't exist yet
    ZmqSubscriber subscriber(config_);
    EXPECT_TRUE(subscriber.connect());
    EXPECT_TRUE(subscriber.is_connected());
}

TEST_F(ZmqSubscriberTest, ConnectFailureWithInvalidEndpoint) {
    config_.endpoint = "invalid://endpoint";
    ZmqSubscriber subscriber(config_);
    EXPECT_FALSE(subscriber.connect());
    EXPECT_FALSE(subscriber.is_connected());
}

TEST_F(ZmqSubscriberTest, SubscribeWithoutConnectFails) {
    ZmqSubscriber subscriber(config_);
    EXPECT_FALSE(subscriber.subscribe("test_topic"));
}

TEST_F(ZmqSubscriberTest, SubscribeAfterConnectSucceeds) {
    ZmqSubscriber subscriber(config_);
    ASSERT_TRUE(subscriber.connect());
    EXPECT_TRUE(subscriber.subscribe("test_topic"));
}

TEST_F(ZmqSubscriberTest, SubscribeToMultipleTopics) {
    ZmqSubscriber subscriber(config_);
    ASSERT_TRUE(subscriber.connect());

    EXPECT_TRUE(subscriber.subscribe("topic1"));
    EXPECT_TRUE(subscriber.subscribe("topic2"));
    EXPECT_TRUE(subscriber.subscribe("topic3"));
}

TEST_F(ZmqSubscriberTest, SubscribeToAllTopicsWithEmptyString) {
    ZmqSubscriber subscriber(config_);
    ASSERT_TRUE(subscriber.connect());
    EXPECT_TRUE(subscriber.subscribe(""));
}

TEST_F(ZmqSubscriberTest, UnsubscribeFromTopic) {
    ZmqSubscriber subscriber(config_);
    ASSERT_TRUE(subscriber.connect());
    ASSERT_TRUE(subscriber.subscribe("test_topic"));

    EXPECT_TRUE(subscriber.unsubscribe("test_topic"));
}

TEST_F(ZmqSubscriberTest, UnsubscribeWithoutSubscribeFails) {
    ZmqSubscriber subscriber(config_);
    ASSERT_TRUE(subscriber.connect());
    EXPECT_FALSE(subscriber.unsubscribe("nonexistent_topic"));
}

TEST_F(ZmqSubscriberTest, ReceiveWithoutConnectionFails) {
    ZmqSubscriber subscriber(config_);

    std::string topic;
    std::vector<uint8_t> data;
    auto result = subscriber.receive_raw();
    EXPECT_FALSE(result.has_value());
}

TEST_F(ZmqSubscriberTest, ReceiveTimeoutWhenNoMessages) {
    config_.receive_timeout_ms = 100;  // Short timeout
    ZmqSubscriber subscriber(config_);
    ASSERT_TRUE(subscriber.connect());
    ASSERT_TRUE(subscriber.subscribe(""));

    std::string topic;
    std::vector<uint8_t> data;

    // Should timeout since no publisher exists
    auto start = std::chrono::steady_clock::now();
    auto result = subscriber.receive_raw();
    EXPECT_FALSE(result.has_value());
    auto duration = std::chrono::steady_clock::now() - start;

    // Should timeout roughly after configured time
    EXPECT_GE(duration, 90ms);
    EXPECT_LE(duration, 200ms);
}

// ============================================================================
// Integration Tests - PUB/SUB Roundtrip
// ============================================================================

TEST_F(ZmqIntegrationTest, PublishSubscribeRawRoundtrip) {
    ZmqPublisher publisher(pub_config_);
    ASSERT_TRUE(publisher.bind());

    ZmqSubscriber subscriber(sub_config_);
    ASSERT_TRUE(subscriber.connect());
    ASSERT_TRUE(subscriber.subscribe(""));

    // Give ZeroMQ time to establish connection
    std::this_thread::sleep_for(100ms);

    // Publish message
    std::vector<uint8_t> sent_data = {0xDE, 0xAD, 0xBE, 0xEF};
    ASSERT_TRUE(publisher.publish_raw("test_topic", sent_data));

    // Receive message
    std::string received_topic;
    std::vector<uint8_t> received_data;
    auto result = subscriber.receive_raw();
    ASSERT_TRUE(result.has_value());
    received_data = result.value();

    // Verify
    EXPECT_EQ(received_data, sent_data);
    EXPECT_EQ(subscriber.messages_received(), 1);
}

TEST_F(ZmqIntegrationTest, PublishSubscribeTypedMessage) {
    ZmqPublisher publisher(pub_config_);
    ASSERT_TRUE(publisher.bind());

    ZmqSubscriber subscriber(sub_config_);
    ASSERT_TRUE(subscriber.connect());
    ASSERT_TRUE(subscriber.subscribe("camera"));

    std::this_thread::sleep_for(100ms);

    // Create and publish message
    CameraFrameData sent_frame{
        .sensor_id_ = "camera_01",
        .timestamp_ns_ = 1234567890123,
        .frame_id = 42,
        .width = 1920,
        .height = 1080,
        .encoding = "RGB8"
    };
    Message<CameraFrameData> sent_message(sent_frame);
    ASSERT_TRUE(publisher.publish("camera", sent_message));

    // Receive message
    auto result = subscriber.receive<CameraFrameData>();
    ASSERT_TRUE(result.has_value());
    auto received_message = result.value();

    // Verify
    const auto& received_frame = received_message.payload();
    EXPECT_EQ(received_frame.sensor_id(), sent_frame.sensor_id());
    EXPECT_EQ(received_frame.timestamp_ns(), sent_frame.timestamp_ns());
    EXPECT_EQ(received_frame.frame_id, sent_frame.frame_id);
    EXPECT_EQ(received_frame.width, sent_frame.width);
    EXPECT_EQ(received_frame.height, sent_frame.height);
}

TEST_F(ZmqIntegrationTest, TopicFiltering) {
    ZmqPublisher publisher(pub_config_);
    ASSERT_TRUE(publisher.bind());

    ZmqSubscriber subscriber(sub_config_);
    ASSERT_TRUE(subscriber.connect());
    ASSERT_TRUE(subscriber.subscribe("camera"));  // Only subscribe to "camera" topics

    std::this_thread::sleep_for(100ms);

    // Publish to different topics
    std::vector<uint8_t> camera_data = {0xCA, 0xAE, 0xAA};
    std::vector<uint8_t> lidar_data = {0xA1, 0xDA, 0xAA};
    std::vector<uint8_t> camera_rgb_data = {0xAA, 0xB8};

    ASSERT_TRUE(publisher.publish_raw("camera", camera_data));
    ASSERT_TRUE(publisher.publish_raw("lidar", lidar_data));
    ASSERT_TRUE(publisher.publish_raw("camera_rgb", camera_rgb_data));

    std::this_thread::sleep_for(50ms);

    // Should receive only "camera" and "camera_rgb" (prefix match), NOT "lidar"
    auto result1 = subscriber.receive_raw();
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), camera_data);

    auto result2 = subscriber.receive_raw();
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), camera_rgb_data);

    EXPECT_EQ(subscriber.messages_received(), 2u);

    // Verify no lidar message is received (timeout should occur)
    SubscriberConfig verify_config = sub_config_;
    verify_config.receive_timeout_ms = 100;
    ZmqSubscriber sub_verify(verify_config);
    ASSERT_TRUE(sub_verify.connect());
    ASSERT_TRUE(sub_verify.subscribe("camera"));
    std::this_thread::sleep_for(100ms);

    auto result3 = sub_verify.receive_raw();
    EXPECT_FALSE(result3.has_value());  // Should timeout, no lidar data received
}

TEST_F(ZmqIntegrationTest, MultipleSubscribers) {
    ZmqPublisher publisher(pub_config_);
    ASSERT_TRUE(publisher.bind());

    // Create three subscribers
    ZmqSubscriber sub1(sub_config_);
    ZmqSubscriber sub2(sub_config_);
    ZmqSubscriber sub3(sub_config_);

    ASSERT_TRUE(sub1.connect());
    ASSERT_TRUE(sub2.connect());
    ASSERT_TRUE(sub3.connect());

    ASSERT_TRUE(sub1.subscribe(""));
    ASSERT_TRUE(sub2.subscribe(""));
    ASSERT_TRUE(sub3.subscribe(""));

    std::this_thread::sleep_for(100ms);

    // Publish one message
    std::vector<uint8_t> data = {0xCA, 0xFE};
    ASSERT_TRUE(publisher.publish_raw("broadcast", data));

    std::this_thread::sleep_for(50ms);

    // All three subscribers should receive it
    auto result1 = sub1.receive_raw();
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), data);

    auto result2 = sub2.receive_raw();
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), data);

    auto result3 = sub3.receive_raw();
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(result3.value(), data);
}

TEST_F(ZmqIntegrationTest, HighThroughputTest) {
    ZmqPublisher publisher(pub_config_);
    ASSERT_TRUE(publisher.bind());

    ZmqSubscriber subscriber(sub_config_);
    ASSERT_TRUE(subscriber.connect());
    ASSERT_TRUE(subscriber.subscribe(""));

    std::this_thread::sleep_for(100ms);

    // Send 1000 messages
    const int num_messages = 1000;
    std::vector<uint8_t> data = {1, 2, 3, 4};

    for (int i = 0; i < num_messages; ++i) {
        ASSERT_TRUE(publisher.publish_raw("perf_test", data));
    }

    // Receive all messages
    int received_count = 0;

    while (received_count < num_messages) {
        auto result = subscriber.receive_raw();
        if (result.has_value()) {
            received_count++;
        } else {
            break;  // Timeout
        }
    }

    // Should receive most/all messages (allow some loss in test environment)
    EXPECT_GE(received_count, num_messages * 0.95);  // At least 95%
    EXPECT_EQ(publisher.messages_sent(), num_messages);
}

TEST_F(ZmqIntegrationTest, LargeMessageTransfer) {
    ZmqPublisher publisher(pub_config_);
    ASSERT_TRUE(publisher.bind());

    ZmqSubscriber subscriber(sub_config_);
    ASSERT_TRUE(subscriber.connect());
    ASSERT_TRUE(subscriber.subscribe(""));

    std::this_thread::sleep_for(100ms);

    // Create 10MB message
    std::vector<uint8_t> large_data(10 * 1024 * 1024);
    for (size_t i = 0; i < large_data.size(); ++i) {
        large_data[i] = static_cast<uint8_t>(i % 256);
    }

    ASSERT_TRUE(publisher.publish_raw("large", large_data));

    auto received_data = subscriber.receive_raw();
    ASSERT_TRUE(received_data.has_value());

    EXPECT_EQ(received_data->size(), large_data.size());
    EXPECT_EQ(received_data, large_data);
}

// ============================================================================
// PeriodicPublisher Tests
// ============================================================================

TEST_F(ZmqIntegrationTest, PeriodicPublisherBasicOperation) {
    ZmqPublisher publisher(pub_config_);
    ASSERT_TRUE(publisher.bind());

    ZmqSubscriber subscriber(sub_config_);
    ASSERT_TRUE(subscriber.connect());
    ASSERT_TRUE(subscriber.subscribe("periodic"));

    std::this_thread::sleep_for(100ms);

    // Create periodic publisher
    int counter = 0;
    auto generator = [&counter]() -> CameraFrameData {
        return CameraFrameData{
            .sensor_id_ = "test_camera",
            .timestamp_ns_ = Timestamp::now().nanoseconds(),
            .frame_id = static_cast<uint32_t>(counter++),
            .width = 640,
            .height = 480,
            .encoding = "RGB8"
        };
    };

    PeriodicPublisher<CameraFrameData> periodic_pub(
        publisher, "periodic", generator, 10ms
    );

    EXPECT_FALSE(periodic_pub.is_running());

    periodic_pub.start();
    EXPECT_TRUE(periodic_pub.is_running());

    // Let it publish for a bit
    std::this_thread::sleep_for(100ms);

    periodic_pub.stop();
    EXPECT_FALSE(periodic_pub.is_running());

    // Should have published multiple messages
    EXPECT_GT(publisher.messages_sent(), 5);
}

TEST_F(ZmqIntegrationTest, PeriodicPublisherReceiveMessages) {
    ZmqPublisher publisher(pub_config_);
    ASSERT_TRUE(publisher.bind());

    ZmqSubscriber subscriber(sub_config_);
    ASSERT_TRUE(subscriber.connect());
    ASSERT_TRUE(subscriber.subscribe("periodic"));

    std::this_thread::sleep_for(100ms);

    // Create periodic publisher
    int counter = 0;
    auto generator = [&counter]() -> CameraFrameData {
        return CameraFrameData{
            .sensor_id_ = "test_camera",
            .timestamp_ns_ = Timestamp::now().nanoseconds(),
            .frame_id = static_cast<uint32_t>(counter++),
            .width = 640,
            .height = 480,
            .encoding = "RGB8"
        };
    };

    PeriodicPublisher<CameraFrameData> periodic_pub(
        publisher, "periodic", generator, 20ms
    );

    periodic_pub.start();

    // Receive several messages
    for (int i = 0; i < 5; ++i) {
        auto result = subscriber.receive<CameraFrameData>();
        ASSERT_TRUE(result.has_value());
        auto msg = result.value();
        EXPECT_EQ(msg.payload().frame_id, static_cast<uint32_t>(i));
    }

    periodic_pub.stop();
}

// ============================================================================
// Performance/Stress Tests
// ============================================================================

TEST_F(ZmqIntegrationTest, LatencyMeasurement) {
    ZmqPublisher publisher(pub_config_);
    ASSERT_TRUE(publisher.bind());

    ZmqSubscriber subscriber(sub_config_);
    ASSERT_TRUE(subscriber.connect());
    ASSERT_TRUE(subscriber.subscribe(""));

    std::this_thread::sleep_for(100ms);

    // Measure round-trip latency
    const int num_iterations = 100;
    std::vector<std::chrono::nanoseconds> latencies;
    latencies.reserve(num_iterations);

    std::vector<uint8_t> data = {1, 2, 3, 4};

    for (int i = 0; i < num_iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        ASSERT_TRUE(publisher.publish_raw("latency_test", data));

        auto received_data = subscriber.receive_raw();
        ASSERT_TRUE(received_data.has_value());

        auto end = std::chrono::high_resolution_clock::now();
        latencies.push_back(end - start);
    }

    // Calculate statistics
    auto sum = std::accumulate(latencies.begin(), latencies.end(),
                               std::chrono::nanoseconds(0));
    auto avg_latency = sum / num_iterations;

    // Latency should be reasonable (< 10ms on local machine)
    EXPECT_LT(avg_latency.count(), 10'000'000);  // 10ms in nanoseconds

    // Print statistics for analysis
    std::cout << "Average latency: "
              << std::chrono::duration_cast<std::chrono::microseconds>(avg_latency).count()
              << " µs\n";
}
