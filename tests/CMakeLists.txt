# Test suite for SensorStreamKit

find_package(GTest REQUIRED)

# ============================================================================
# Message Class Tests
# ============================================================================

add_executable(test_message_class
    test_message_class.cpp
)

target_link_libraries(test_message_class
    PRIVATE
        sensorstreamkit
    GTest::gtest_main
)

target_compile_features(test_message_class PRIVATE cxx_std_20)

# Add test to CTest
add_test(NAME MessageClassTests COMMAND test_message_class)

# ============================================================================
# SequenceCounter Tests (PIMPL Pattern)
# ============================================================================

add_executable(test_sequence_counter
    test_sequence_counter.cpp
)

target_link_libraries(test_sequence_counter
    PRIVATE
        sensorstreamkit
    GTest::gtest_main
)

target_compile_features(test_sequence_counter PRIVATE cxx_std_20)

# Add test to CTest
add_test(NAME SequenceCounterTests COMMAND test_sequence_counter)

# ============================================================================
# ZMQ Transport Tests
# ============================================================================

add_executable(test_zmq_transport
    test_zmq_transport.cpp
)

target_link_libraries(test_zmq_transport
    PRIVATE
        sensorstreamkit
    GTest::gtest_main
)

target_compile_features(test_zmq_transport PRIVATE cxx_std_20)

# Add test to CTest
add_test(NAME ZmqTransportTests COMMAND test_zmq_transport)

# ============================================================================
# Additional compiler flags for tests
# ============================================================================

target_compile_options(test_message_class PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

target_compile_options(test_sequence_counter PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

target_compile_options(test_zmq_transport PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)
