cmake_minimum_required(VERSION 3.20)

project(SensorStreamKit
    VERSION 1.0.0
    DESCRIPTION "Modern C++20 library for real-time sensor data streaming"
    HOMEPAGE_URL "https://github.com/coolwindjo/SensorStreamKit"
    LANGUAGES CXX
)

# ============================================================================
# C++20 Requirements
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ============================================================================
# Options
# ============================================================================
option(SENSORSTREAMKIT_BUILD_EXAMPLES "Build examples" ON)
option(SENSORSTREAMKIT_BUILD_TESTS "Build tests" ON)
option(SENSORSTREAMKIT_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(SENSORSTREAMKIT_ENABLE_COVERAGE "Enable code coverage" OFF)

# ============================================================================
# Dependencies via vcpkg
# ============================================================================
find_package(cppzmq REQUIRED)
find_package(flatbuffers REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(httplib REQUIRED)

# ============================================================================
# Main Library
# ============================================================================
add_library(sensorstreamkit
    src/sensorstreamkit/core/message.cpp
    src/sensorstreamkit/transport/zmq_publisher.cpp
    src/sensorstreamkit/transport/zmq_subscriber.cpp
    src/sensorstreamkit/transport/zmq_transport.cpp
#     src/core/serialization.cpp
#     src/api/rest_server.cpp
#     src/sensors/camera_sensor.cpp
#     src/sensors/lidar_sensor.cpp
#     src/sensors/imu_sensor.cpp
)

target_include_directories(sensorstreamkit
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(sensorstreamkit
    PUBLIC
        cppzmq
        flatbuffers::flatbuffers
        nlohmann_json::nlohmann_json
        httplib::httplib
)

target_compile_features(sensorstreamkit PUBLIC cxx_std_20)

# Compiler warnings
target_compile_options(sensorstreamkit PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Werror>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Werror>
)

# Debug/Release specific options
target_compile_options(sensorstreamkit PRIVATE
    $<$<CONFIG:Debug>:-g3 -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:-g -O2>
)

# ============================================================================
# FlatBuffers Schema Generation
# ============================================================================
set(FLATBUFFERS_SCHEMAS
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/sensor_data.fbs
)

# Generate FlatBuffers headers (run manually or add custom command)
# flatc --cpp -o ${CMAKE_CURRENT_SOURCE_DIR}/include/sensorstreamkit/generated ${FLATBUFFERS_SCHEMAS}

# ============================================================================
# Examples
# ============================================================================
if(SENSORSTREAMKIT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ============================================================================
# Tests
# ============================================================================
if(SENSORSTREAMKIT_BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    add_subdirectory(tests)
endif()

# ============================================================================
# Installation
# ============================================================================
# include(GNUInstallDirs)
# include(CMakePackageConfigHelpers)

# install(TARGETS sensorstreamkit
#     EXPORT SensorStreamKitTargets
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )

# install(DIRECTORY include/sensorstreamkit
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )

# install(EXPORT SensorStreamKitTargets
#     FILE SensorStreamKitTargets.cmake
#     NAMESPACE SensorStreamKit::
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SensorStreamKit
# )

# configure_package_config_file(
#     ${CMAKE_CURRENT_SOURCE_DIR}/cmake/SensorStreamKitConfig.cmake.in
#     ${CMAKE_CURRENT_BINARY_DIR}/SensorStreamKitConfig.cmake
#     INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SensorStreamKit
# )

# write_basic_package_version_file(
#     ${CMAKE_CURRENT_BINARY_DIR}/SensorStreamKitConfigVersion.cmake
#     VERSION ${PROJECT_VERSION}
#     COMPATIBILITY SameMajorVersion
# )

# install(FILES
#     ${CMAKE_CURRENT_BINARY_DIR}/SensorStreamKitConfig.cmake
#     ${CMAKE_CURRENT_BINARY_DIR}/SensorStreamKitConfigVersion.cmake
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SensorStreamKit
# )
