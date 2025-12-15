# ============================================================================
# Examples for SensorStreamKit
# ============================================================================

# 01_simple_publisher
add_executable(simple_publisher
    01_simple_publisher/main.cpp
)

target_link_libraries(simple_publisher
    PRIVATE
        sensorstreamkit
)

# 02_simple_subscriber
add_executable(simple_subscriber
    02_simple_subscriber/main.cpp
)

target_link_libraries(simple_subscriber
    PRIVATE
        sensorstreamkit
)

# Install examples (optional)
install(TARGETS simple_publisher simple_subscriber
    RUNTIME DESTINATION bin/examples
)
