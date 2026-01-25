# ZMQ Transport Test Coverage

## Overview
Comprehensive test-driven development (TDD) test suite for `ZmqPublisher` and `ZmqSubscriber` classes.

## Test Files
- **test_zmq_transport.cpp**: Complete test suite with 40+ test cases

## Test Coverage

### ZmqPublisher Tests (15 tests)

#### Construction & Configuration
- ✅ `ConstructorWithDefaultConfig` - Default initialization
- ✅ `ConstructorWithCustomConfig` - Custom high_water_mark, timeout, conflate options
- ✅ `MoveConstructor` - Move semantics validation
- ✅ `MoveAssignment` - Move assignment validation

#### Binding
- ✅ `BindSuccess` - Successful bind to valid endpoint
- ✅ `BindFailureWithInvalidEndpoint` - Failure with invalid endpoint
- ✅ `BindTwiceFails` - Port already in use detection

#### Publishing
- ✅ `PublishRawWithoutBindFails` - Pre-bind validation
- ✅ `PublishRawAfterBindSucceeds` - Basic publish operation
- ✅ `PublishMultipleMessagesIncrementsCounter` - Counter validation
- ✅ `PublishTypedCameraMessage` - Typed message publishing
- ✅ `PublishEmptyTopic` - Edge case: empty topic string
- ✅ `PublishEmptyData` - Edge case: empty data buffer
- ✅ `PublishLargeMessage` - Stress test: 1MB message

#### Features
- ✅ `ConflateOptionKeepsOnlyLastMessage` - Conflate mode validation
- ✅ `InitialMessageCountIsZero` - Initial state validation

### ZmqSubscriber Tests (12 tests)

#### Construction & Configuration
- ✅ `ConstructorWithDefaultConfig` - Default initialization
- ✅ `ConstructorWithCustomConfig` - Custom configuration
- ✅ `MoveConstructor` - Move semantics validation
- ✅ `MoveAssignment` - Move assignment validation

#### Connection
- ✅ `InitiallyNotConnected` - Initial state
- ✅ `ConnectSuccess` - Successful connection
- ✅ `ConnectFailureWithInvalidEndpoint` - Invalid endpoint handling

#### Subscription
- ✅ `SubscribeWithoutConnectFails` - Pre-connect validation
- ✅ `SubscribeAfterConnectSucceeds` - Basic subscribe operation
- ✅ `SubscribeToMultipleTopics` - Multiple topic subscriptions
- ✅ `SubscribeToAllTopicsWithEmptyString` - Wildcard subscription
- ✅ `UnsubscribeFromTopic` - Unsubscribe operation
- ✅ `UnsubscribeWithoutSubscribeFails` - Unsubscribe validation

#### Receiving
- ✅ `ReceiveWithoutConnectionFails` - Pre-connection validation
- ✅ `ReceiveTimeoutWhenNoMessages` - Timeout behavior
- ✅ `InitialMessageCountIsZero` - Initial state validation

### Integration Tests (10 tests)

#### Basic PUB/SUB
- ✅ `PublishSubscribeRawRoundtrip` - End-to-end raw message transfer
- ✅ `PublishSubscribeTypedMessage` - End-to-end typed message transfer
- ✅ `LargeMessageTransfer` - 10MB message transfer

#### Topic Filtering
- ✅ `TopicFiltering` - Prefix-based topic matching
- ✅ `MultipleSubscribers` - Fan-out pattern (1 pub → 3 subs)

#### Performance
- ✅ `HighThroughputTest` - 1000 messages/sec throughput
- ✅ `LatencyMeasurement` - Round-trip latency measurement

#### PeriodicPublisher
- ✅ `PeriodicPublisherBasicOperation` - Start/stop functionality
- ✅ `PeriodicPublisherReceiveMessages` - Receive from periodic publisher

## Test Execution

### Build and Run All Tests
```bash
cmake --preset debug
cmake --build build
ctest --test-dir build --output-on-failure
```

### Run Specific Test Suite
```bash
build/tests/test_zmq_transport
```

### Run Specific Test
```bash
build/tests/test_zmq_transport --gtest_filter=ZmqIntegrationTest.PublishSubscribeRawRoundtrip
```

### Run with Verbose Output
```bash
build/tests/test_zmq_transport --gtest_print_time=1 --gtest_color=yes
```

## Test Patterns Used

### 1. Fixture Pattern
```cpp
class ZmqPublisherTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Unique port allocation per test
        static int port = 15550;
        config_.endpoint = "tcp://*:" + std::to_string(port++);
    }
    PublisherConfig config_;
};
```

### 2. Integration Test Pattern
```cpp
TEST_F(ZmqIntegrationTest, PublishSubscribeRawRoundtrip) {
    // Setup
    ZmqPublisher publisher(pub_config_);
    ZmqSubscriber subscriber(sub_config_);

    // Execute
    publisher.publish_raw("topic", data);
    subscriber.receive_raw(topic, received_data);

    // Verify
    EXPECT_EQ(received_data, data);
}
```

### 3. Edge Case Testing
- Empty topics, empty data
- Large messages (1MB, 10MB)
- Invalid endpoints
- Operations before initialization

### 4. Performance Testing
- Throughput measurement
- Latency profiling
- High water mark validation

## Coverage Metrics (Target)

- **Line Coverage**: > 90%
- **Branch Coverage**: > 85%
- **Function Coverage**: 100%

## TDD Implementation Order

Following Test-Driven Development:

1. **RED**: Write failing test
2. **GREEN**: Implement minimal code to pass
3. **REFACTOR**: Clean up implementation

### Example TDD Flow:
```cpp
// 1. RED - Test written first (fails)
TEST_F(ZmqPublisherTest, BindSuccess) {
    ZmqPublisher publisher(config_);
    EXPECT_TRUE(publisher.bind());  // Not implemented yet
}

// 2. GREEN - Minimal implementation
bool ZmqPublisher::bind() {
    socket_->bind(config_.endpoint);
    return true;
}

// 3. REFACTOR - Add error handling, atomics, etc.
bool ZmqPublisher::bind() {
    try {
        socket_->bind(config_.endpoint);
        bound_ = true;
        return true;
    } catch (const zmq::error_t&) {
        bound_ = false;
        return false;
    }
}
```

## Running Tests in CI/CD

### GitHub Actions Integration
```yaml
- name: Run Tests
  run: |
    cmake --preset debug
    cmake --build build
    ctest --test-dir build --output-on-failure
```

### Code Coverage
```bash
# With coverage enabled
cmake --preset debug -DSENSORSTREAMKIT_ENABLE_COVERAGE=ON
cmake --build build
ctest --test-dir build
lcov --capture --directory build --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## Known Limitations & Future Tests

### To Be Implemented
- [ ] Security tests (ZMQ CURVE encryption)
- [ ] Connection resilience (auto-reconnect)
- [ ] Multi-threaded concurrent publishing
- [ ] Backpressure handling
- [ ] Message priority queues
- [ ] Benchmarking suite (Google Benchmark)

### Platform-Specific Tests
- [ ] Windows named pipes
- [ ] Unix domain sockets
- [ ] IPv6 endpoints

## Dependencies

- **GoogleTest**: 1.14.0+
- **cppzmq**: 4.10.0+
- **C++20 compiler**: GCC 12+, Clang 16+

## Contributing

When adding new features:
1. Write tests FIRST (TDD)
2. Follow existing test patterns
3. Ensure all tests pass
4. Add test documentation here

## References

- [ZeroMQ Guide](https://zguide.zeromq.org/)
- [GoogleTest Primer](https://google.github.io/googletest/primer.html)
- [Test-Driven Development](https://martinfowler.com/bliki/TestDrivenDevelopment.html)
