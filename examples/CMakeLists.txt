# ============================================================================
# Examples for SensorStreamKit
# ============================================================================
cmake_minimum_required(VERSION 3.16)

# 01_simple_publisher
add_executable(simple_publisher
    01_simple_publisher/main.cpp
)

# Assuming 'sensorstreamkit' library target is available from the root project
target_link_libraries(simple_publisher
    PRIVATE
        sensorstreamkit
)

target_compile_features(simple_publisher PRIVATE cxx_std_20)

# 02_simple_subscriber
add_executable(simple_subscriber
    02_simple_subscriber/main.cpp
)

target_link_libraries(simple_subscriber
    PRIVATE
        sensorstreamkit
)

target_compile_features(simple_subscriber PRIVATE cxx_std_20)

# 03_broker
add_executable(broker
    03_broker/main.cpp
)

target_link_libraries(broker
    PRIVATE
        sensorstreamkit
)

target_compile_features(broker PRIVATE cxx_std_20)

# 04_rest_api
add_executable(rest_api_example
    04_rest_api/main.cpp
)

target_link_libraries(rest_api_example
    PRIVATE
        sensorstreamkit
)

target_compile_features(rest_api_example PRIVATE cxx_std_20)

# 05_integrated_system
add_executable(integrated_system
    05_integrated_system/main.cpp
)

target_link_libraries(integrated_system
    PRIVATE
        sensorstreamkit
)

target_compile_features(integrated_system PRIVATE cxx_std_20)

# Install examples (optional)
install(TARGETS simple_publisher simple_subscriber broker rest_api_example integrated_system
    RUNTIME DESTINATION bin/examples
)
