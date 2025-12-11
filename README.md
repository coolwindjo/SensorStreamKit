# SensorStreamKit

[![CI](https://github.com/coolwindjo/SensorStreamKit/actions/workflows/ci.yml/badge.svg)](https://github.com/coolwindjo/SensorStreamKit/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/coolwindjo/SensorStreamKit/branch/main/graph/badge.svg)](https://codecov.io/gh/coolwindjo/SensorStreamKit)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)

**A modern C++20 library for real-time sensor data streaming with ZeroMQ transport and REST API configuration.**

Designed for rapid prototyping of sensor data pipelines in ADAS, robotics, and autonomous systems applications.

## ✨ Features

- **Modern C++20**: Concepts, coroutines, `std::jthread`, `std::span`, ranges
- **High-Performance Messaging**: ZeroMQ PUB/SUB, REQ/REP patterns
- **REST API**: Configuration and monitoring via HTTP endpoints
- **Zero-Copy Serialization**: FlatBuffers for minimal overhead
- **Docker Ready**: Multi-container deployment with Docker Compose
- **Production Patterns**: From real Mercedes-Benz ADAS development experience

## 🚀 Quick Start

### Prerequisites

- C++20 compiler (GCC 13+, Clang 17+)
- CMake 3.20+
- vcpkg (recommended) or system packages

### Build from Source

```bash
# Clone the repository
git clone https://github.com/coolwindjo/SensorStreamKit.git
cd SensorStreamKit

# Install dependencies via vcpkg (requires VCPKG_ROOT environment variable)
$VCPKG_ROOT/vcpkg install

# Configure and build using presets
cmake --preset release
cmake --build --preset release --parallel

# Run tests
ctest --preset release
```

### Docker Development

```bash
# Build development image (use --platform for cross-architecture builds)
# On Apple Silicon (ARM64), specify linux/amd64 for x64 target:
docker build --platform linux/amd64 -f docker/Dockerfile.dev --target dev -t sensorstreamkit-dev .

# Or use docker compose (platform is pre-configured in docker-compose.yml)
cd docker
docker compose up -d dev

# Attach to container
docker exec -it sensorstreamkit-dev bash

# Inside container: build and test using presets
cmake --preset debug
cmake --build --preset debug --parallel
ctest --preset debug
```

> **Note**: The docker-compose.yml is configured with `platform: linux/amd64` for all services to ensure consistent builds across different host architectures (including Apple Silicon Macs).

### Run Full Demo

```bash
# Start sensor server and client
docker compose up sensor-server sensor-client

# In another terminal: check REST API
curl http://localhost:8080/health
curl http://localhost:8080/sensors
```

## 📖 Usage Examples

### Basic Publisher

```cpp
#include "sensorstreamkit/core/message.hpp"
#include "sensorstreamkit/transport/zmq_publisher.hpp"

using namespace sensorstreamkit;
using namespace sensorstreamkit::transport;

int main() {
    // Configure publisher
    PublisherConfig config{
        .endpoint = "tcp://*:5555",
        .high_water_mark = 1000,
        .send_timeout_ms = 1000
    };

    // Create and bind publisher
    ZmqPublisher publisher(config);
    if (!publisher.bind()) {
        return 1;
    }

    // Publish camera data
    CameraFrameData frame{
        .sensor_id_ = "camera_front",
        .timestamp_ns_ = Timestamp::now().nanoseconds(),
        .frame_id = 1,
        .width = 1920,
        .height = 1080,
        .encoding = "RGB8"
    };

    Message<CameraFrameData> msg(frame);
    publisher.publish("camera", msg);

    return 0;
}
```

### Periodic Publisher with C++20 jthread

```cpp
#include "sensorstreamkit/core/message.hpp"
#include "sensorstreamkit/transport/zmq_publisher.hpp"
#include <thread>

using namespace sensorstreamkit;
using namespace sensorstreamkit::transport;

int main() {
    PublisherConfig config{.endpoint = "tcp://*:5555"};
    ZmqPublisher publisher(config);
    if (!publisher.bind()) {
        return 1;
    }

    // Generator function for simulated IMU data
    auto imu_generator = []() -> ImuData {
        return ImuData{
            .sensor_id_ = "imu_main",
            .timestamp_ns_ = Timestamp::now().nanoseconds(),
            .accel_x = 0.0f,
            .accel_y = 0.0f,
            .accel_z = 9.81f,
            .gyro_x = 0.0f,
            .gyro_y = 0.0f,
            .gyro_z = 0.0f
        };
    };

    // Start periodic publishing at 100Hz
    PeriodicPublisher<ImuData> imu_pub(
        publisher, "imu", imu_generator,
        std::chrono::milliseconds(10)
    );
    imu_pub.start();

    // jthread automatically stops on destruction
    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}
```

### REST API Configuration (Planned)

> **Note**: REST API functionality is planned for a future release.

```cpp
// Example of planned REST API (not yet implemented)
#include <sensorstreamkit/api/rest_server.hpp>

using namespace sensorstreamkit::api;

int main() {
    RestServer server(8080);

    // Health check endpoint
    server.get("/health", [](const Request&) {
        return Response::json({{"status", "ok"}});
    });

    // Get sensor list
    server.get("/sensors", [](const Request&) {
        return Response::json({
            {"sensors", {"camera_front", "lidar_top", "imu_main"}}
        });
    });

    // Blocks until stopped
    server.run();

    return 0;
}
```

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      REST API (Port 8080)                   │
│  GET /health  │  GET /sensors  │  POST /sensors/:id/start   │
└────────────────────────────┬────────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────────┐
│                    SensorStreamKit Core                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐          │
│  │   Camera    │  │    LiDAR    │  │     IMU     │          │
│  │   Sensor    │  │   Sensor    │  │   Sensor    │          │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘          │
│         │                │                │                 │
│  ┌──────▼────────────────▼────────────────▼──────┐          │
│  │           FlatBuffers Serialization           │          │
│  └──────────────────────┬────────────────────────┘          │
│                         │                                   │
│  ┌──────────────────────▼────────────────────────┐          │
│  │              ZeroMQ PUB Socket                │          │
│  │              (tcp://*:5555)                   │          │
│  └──────────────────────┬────────────────────────┘          │
└─────────────────────────┼───────────────────────────────────┘
                          │
            ┌─────────────┼─────────────┐
            ▼             ▼             ▼
     ┌──────────┐  ┌──────────┐  ┌──────────┐
     │ Client 1 │  │ Client 2 │  │ Client N │
     │ (camera) │  │ (lidar)  │  │  (all)   │
     └──────────┘  └──────────┘  └──────────┘
```

## 📁 Project Structure

```
SensorStreamKit/
├── include/sensorstreamkit/    # Public headers
│   ├── core/                   # Message types, serialization
│   ├── transport/              # ZeroMQ wrappers
│   ├── api/                    # REST server/client (planned)
│   └── sensors/                # Sensor abstractions (planned)
├── src/                        # Implementation
├── examples/                   # Usage examples
│   ├── 01_simple_publisher/
│   └── 02_simple_subscriber/
├── tests/                      # Unit & integration tests
├── docker/                     # Containerization
├── docs/                       # Documentation
└── proto/                      # FlatBuffers schemas
```

## 🔧 Configuration

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `ZMQ_PUB_ENDPOINT` | `tcp://*:5555` | ZeroMQ publisher endpoint |
| `ZMQ_HIGH_WATER_MARK` | `1000` | Max queued messages |
| `REST_PORT` | `8080` | REST API port |
| `SENSOR_CAMERA_RATE_HZ` | `30` | Camera publish rate |
| `SENSOR_LIDAR_RATE_HZ` | `10` | LiDAR publish rate |
| `SENSOR_IMU_RATE_HZ` | `100` | IMU publish rate |

## 📊 Performance

Benchmarked on Ubuntu 24.04, AMD Ryzen 9 5900X, GCC 13.2:

| Metric | Value |
|--------|-------|
| Serialization (FlatBuffers) | ~50ns per message |
| ZeroMQ throughput (local) | >500,000 msg/sec |
| End-to-end latency | <1ms (same host) |
| Memory per publisher | ~2MB |

## 🤝 Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## 📄 License

MIT License - see [LICENSE](LICENSE) for details.

## 👤 Author

**Jo, SeungHyeon (Jo,SH)**

- Senior Software Engineer with 12+ years in safety-critical embedded systems
- Former Multi-Sensor Integration Lead for Mercedes-Benz MPC 5.5 (AutoSens 2020 Award)
- [LinkedIn](https://www.linkedin.com/in/josh-auto/)
- [GitHub](https://github.com/coolwindjo)

## 🙏 Acknowledgments

- [cppzmq](https://github.com/zeromq/cppzmq) - ZeroMQ C++ bindings
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - Header-only HTTP library
- [FlatBuffers](https://github.com/google/flatbuffers) - Efficient serialization
- Mercedes-Benz & LG Electronics MPC 5.5 team for the foundation in real-time sensor systems
