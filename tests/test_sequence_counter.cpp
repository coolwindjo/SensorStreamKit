/**
 * @file test_sequence_counter.cpp
 * @brief Unit tests for SequenceCounter PIMPL implementation
 * @author Jo, SeungHyeon (Jo,SH)
 * @date 2025
 *
 * Focuses on:
 * - PIMPL idiom correctness (incomplete type handling, common traps)
 * - Thread-safety verification
 * - Move semantics and noexcept guarantees
 * - Integration with Message<T>
 */

#include <gtest/gtest.h>
#include "sensorstreamkit/core/message.hpp"
#include <thread>
#include <vector>
#include <algorithm>
#include <set>
#include <chrono>

using namespace sensorstreamkit::core;

// ============================================================================
// Test Fixture
// ============================================================================

class SequenceCounterTest : public ::testing::Test {};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST_F(SequenceCounterTest, SequentialIncrement) {
    SequenceCounter counter;

    // Verify sequential increment and uniqueness
    EXPECT_EQ(counter.next(), 0);
    EXPECT_EQ(counter.next(), 1);
    EXPECT_EQ(counter.next(), 2);
    EXPECT_EQ(counter.next(), 3);
}

TEST_F(SequenceCounterTest, UniqueValuesGeneration) {
    SequenceCounter counter;
    std::set<uint32_t> seen_values;

    constexpr int num_calls = 10000;
    for (int i = 0; i < num_calls; ++i) {
        EXPECT_TRUE(seen_values.insert(counter.next()).second);
    }

    EXPECT_EQ(seen_values.size(), num_calls);
}

// ============================================================================
// PIMPL Idiom Correctness Tests
// ============================================================================

TEST_F(SequenceCounterTest, PimplMultipleIndependentInstances) {
    // Each instance should have its own Impl state
    SequenceCounter counter1;
    SequenceCounter counter2;

    EXPECT_EQ(counter1.next(), 0);
    EXPECT_EQ(counter2.next(), 0);
    EXPECT_EQ(counter1.next(), 1);
    EXPECT_EQ(counter2.next(), 1);
}

TEST_F(SequenceCounterTest, PimplWorksWithStlContainers) {
    // PIMPL pattern should work correctly with STL containers
    std::vector<SequenceCounter> counters;

    for (int i = 0; i < 5; ++i) {
        counters.emplace_back();
    }

    // Each counter is independent
    for (auto& counter : counters) {
        EXPECT_EQ(counter.next(), 0);
    }
}

// ============================================================================
// PIMPL Trap: Incomplete Type Tests
// ============================================================================

TEST_F(SequenceCounterTest, PimplTrap_DestructorDefinedInCpp) {
    // CRITICAL: Destructor must be defined in .cpp where Impl is complete
    // If defaulted in header: "error: invalid application of 'sizeof' to incomplete type"

    SequenceCounter* ptr = new SequenceCounter();
    ptr->next();

    EXPECT_NO_THROW({
        delete ptr;  // Would fail to compile if destructor in header
    });
}

TEST_F(SequenceCounterTest, PimplTrap_UniquePtrDeleterRequiresCompleteness) {
    // unique_ptr's default_delete requires complete type at destruction point
    // This is why destructor MUST be in .cpp file

    auto ptr = std::make_unique<SequenceCounter>();
    ptr->next();

    EXPECT_NO_THROW({
        ptr.reset();  // Calls destructor, which must see complete Impl
    });
}

// ============================================================================
// Move Semantics and Special Member Functions
// ============================================================================

TEST_F(SequenceCounterTest, MoveSemantics) {
    SequenceCounter counter1;
    counter1.next();  // 0
    counter1.next();  // 1

    SequenceCounter counter2(std::move(counter1));
    EXPECT_EQ(counter2.next(), 2);

    SequenceCounter counter3;
    counter3 = std::move(counter2);
    EXPECT_EQ(counter3.next(), 3);
}

TEST_F(SequenceCounterTest, MoveIsNoexceptForVectorOptimization) {
    // noexcept move enables vector to use move instead of copy during reallocation
    EXPECT_TRUE(std::is_nothrow_move_constructible_v<SequenceCounter>);
    EXPECT_TRUE(std::is_nothrow_move_assignable_v<SequenceCounter>);
}

TEST_F(SequenceCounterTest, CopyOperationsDeleted) {
    // Atomic counter cannot be safely copied
    EXPECT_FALSE(std::is_copy_constructible_v<SequenceCounter>);
    EXPECT_FALSE(std::is_copy_assignable_v<SequenceCounter>);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(SequenceCounterTest, ConcurrentAccessProducesUniqueValues) {
    SequenceCounter counter;
    constexpr int num_threads = 10;
    constexpr int calls_per_thread = 1000;

    std::vector<std::thread> threads;
    std::vector<std::vector<uint32_t>> all_values(num_threads);

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < calls_per_thread; ++i) {
                all_values[t].push_back(counter.next());
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Collect and verify all values are unique
    std::vector<uint32_t> all_nums;
    for (const auto& vec : all_values) {
        all_nums.insert(all_nums.end(), vec.begin(), vec.end());
    }

    std::sort(all_nums.begin(), all_nums.end());
    auto duplicate = std::adjacent_find(all_nums.begin(), all_nums.end());

    EXPECT_EQ(duplicate, all_nums.end())
        << "Thread-safety violated: duplicate value " << *duplicate;
    EXPECT_EQ(all_nums.size(), num_threads * calls_per_thread);
}

// ============================================================================
// noexcept Guarantees
// ============================================================================

TEST_F(SequenceCounterTest, NoexceptGuarantees) {
    // Critical for use in destructors and move operations
    EXPECT_TRUE(std::is_nothrow_destructible_v<SequenceCounter>);

    // next() should never throw (atomic operations are noexcept)
    SequenceCounter counter;
    EXPECT_NO_THROW({
        for (int i = 0; i < 1000; ++i) {
            counter.next();
        }
    });
}

// ============================================================================
// Integration with Message<T>
// ============================================================================

TEST_F(SequenceCounterTest, MessageTemplateUsesStaticCounter) {
    // Each Message<T> template instantiation has its own static SequenceCounter

    ImuData imu_data{.sensor_id_ = "imu", .timestamp_ns_ = 1};
    CameraFrameData cam_data{
        .sensor_id_ = "cam",
        .timestamp_ns_ = 2,
        .frame_id = 1,
        .width = 640,
        .height = 480,
        .encoding = "RGB8"
    };

    Message<ImuData> imu_msg1(imu_data);
    Message<CameraFrameData> cam_msg1(cam_data);
    Message<ImuData> imu_msg2(imu_data);
    Message<CameraFrameData> cam_msg2(cam_data);

    // Each type has independent sequence
    EXPECT_LT(imu_msg1.header().sequence_number, imu_msg2.header().sequence_number);
    EXPECT_LT(cam_msg1.header().sequence_number, cam_msg2.header().sequence_number);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
