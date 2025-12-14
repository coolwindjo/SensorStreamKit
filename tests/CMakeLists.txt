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
        GTest::gtest
        GTest::gtest_main
)

target_compile_features(test_message_class PRIVATE cxx_std_20)

# Add test to CTest
add_test(NAME MessageClassTests COMMAND test_message_class)

# ============================================================================
# Additional compiler flags for tests
# ============================================================================

target_compile_options(test_message_class PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)
