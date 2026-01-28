#include <gtest/gtest.h>
#include "sensorstreamkit/sensors/sensor_manager.hpp"
#include "sensorstreamkit/sensors/camera_sensor.hpp"
#include "sensorstreamkit/transport/zmq_publisher.hpp"
#include <memory>
#include <thread>

using namespace sensorstreamkit::sensors;
using namespace sensorstreamkit::transport;

class SensorSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        PublisherConfig config{.endpoint = "inproc://test_pipeline"};
        publisher_ = std::make_unique<ZmqPublisher>(config);
        manager_ = std::make_unique<SensorManager>();
    }

    std::unique_ptr<ZmqPublisher> publisher_;
    std::unique_ptr<SensorManager> manager_;
};

TEST_F(SensorSystemTest, ManagerRegistrationAndLifecycle) {
    auto cam = std::make_shared<CameraSensor>("cam1", *publisher_, std::chrono::milliseconds(10));
    manager_->add_sensor(cam);

    EXPECT_EQ(manager_->get_all_sensors().size(), 1);
    EXPECT_EQ(manager_->get_sensor("cam1")->id(), "cam1");
    EXPECT_FALSE(cam->is_active());

    manager_->start_all();
    EXPECT_TRUE(cam->is_active());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    manager_->stop_all();
    EXPECT_FALSE(cam->is_active());
}

TEST_F(SensorSystemTest, MultipleSensorsIndependentControl) {
    auto cam = std::make_shared<CameraSensor>("cam_front", *publisher_);
    auto cam2 = std::make_shared<CameraSensor>("cam_rear", *publisher_);
    
    manager_->add_sensor(cam);
    manager_->add_sensor(cam2);

    cam->start();
    EXPECT_TRUE(cam->is_active());
    EXPECT_FALSE(cam2->is_active());

    cam2->start();
    EXPECT_TRUE(cam2->is_active());

    cam->stop();
    EXPECT_FALSE(cam->is_active());
    EXPECT_TRUE(cam2->is_active());
}
