// Unit tests for MemoryManager.

#include <catch2/catch_test_macros.hpp>

#include <data/Dataset.h>
#include <data/MemoryManager.h>

using namespace lumen::data;

TEST_CASE("MemoryManager -- default budget is 4 GB", "[memory_manager]") {
    auto& mm = MemoryManager::instance();
    mm.reset();

    REQUIRE(mm.memoryBudgetBytes() == 4ULL * 1024 * 1024 * 1024);
}

TEST_CASE("MemoryManager -- decide InMemory for small data", "[memory_manager]") {
    auto& mm = MemoryManager::instance();
    mm.reset();

    auto mode = mm.decide(1024); // 1 KB
    REQUIRE(mode == Dataset::StorageMode::InMemory);
}

TEST_CASE("MemoryManager -- decide Chunked for large data", "[memory_manager]") {
    auto& mm = MemoryManager::instance();
    mm.reset();

    auto mode = mm.decide(200ULL * 1024 * 1024); // 200 MB
    REQUIRE(mode == Dataset::StorageMode::Chunked);
}

TEST_CASE("MemoryManager -- decide threshold boundary", "[memory_manager]") {
    auto& mm = MemoryManager::instance();
    mm.reset();

    // Exactly at threshold -> InMemory
    auto modeAt = mm.decide(MemoryManager::kChunkThreshold);
    REQUIRE(modeAt == Dataset::StorageMode::InMemory);

    // Just above threshold -> Chunked
    auto modeAbove = mm.decide(MemoryManager::kChunkThreshold + 1);
    REQUIRE(modeAbove == Dataset::StorageMode::Chunked);
}

TEST_CASE("MemoryManager -- track allocation and deallocation", "[memory_manager]") {
    auto& mm = MemoryManager::instance();
    mm.reset();

    REQUIRE(mm.currentUsageBytes() == 0);

    mm.trackAllocation(1000);
    REQUIRE(mm.currentUsageBytes() == 1000);

    mm.trackAllocation(2000);
    REQUIRE(mm.currentUsageBytes() == 3000);

    mm.trackDeallocation(1500);
    REQUIRE(mm.currentUsageBytes() == 1500);

    mm.trackDeallocation(1500);
    REQUIRE(mm.currentUsageBytes() == 0);
}

TEST_CASE("MemoryManager -- deallocation does not underflow", "[memory_manager]") {
    auto& mm = MemoryManager::instance();
    mm.reset();

    mm.trackAllocation(100);
    mm.trackDeallocation(200); // More than allocated
    REQUIRE(mm.currentUsageBytes() == 0);
}

TEST_CASE("MemoryManager -- setBudget", "[memory_manager]") {
    auto& mm = MemoryManager::instance();
    mm.reset();

    mm.setBudget(1024 * 1024); // 1 MB
    REQUIRE(mm.memoryBudgetBytes() == 1024 * 1024);

    mm.reset(); // Restore default
    REQUIRE(mm.memoryBudgetBytes() == 4ULL * 1024 * 1024 * 1024);
}

TEST_CASE("MemoryManager -- wouldExceedBudget", "[memory_manager]") {
    auto& mm = MemoryManager::instance();
    mm.reset();

    mm.setBudget(1000);
    REQUIRE_FALSE(mm.wouldExceedBudget(500));
    REQUIRE_FALSE(mm.wouldExceedBudget(1000));
    REQUIRE(mm.wouldExceedBudget(1001));

    mm.trackAllocation(600);
    REQUIRE_FALSE(mm.wouldExceedBudget(400));
    REQUIRE(mm.wouldExceedBudget(401));

    mm.reset();
}

TEST_CASE("MemoryManager -- kChunkThreshold is 100 MB", "[memory_manager]") {
    REQUIRE(MemoryManager::kChunkThreshold == 100ULL * 1024 * 1024);
}
