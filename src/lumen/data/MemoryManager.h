#pragma once

#include <data/Dataset.h>

#include <atomic>
#include <cstddef>

namespace lumen::data {

/// Singleton that manages memory budgets for dataset loading.
///
/// Decides whether new data should be stored InMemory or Chunked
/// based on estimated size vs. a configurable threshold (100 MB).
/// Tracks current memory usage against a configurable budget (default 4 GB).
class MemoryManager {
public:
    /// Access the singleton instance.
    static MemoryManager& instance();

    /// Decide storage mode based on estimated data size.
    /// Returns Chunked if estimatedBytes exceeds the chunk threshold.
    [[nodiscard]] Dataset::StorageMode decide(std::size_t estimatedBytes) const;

    /// Current memory budget in bytes.
    [[nodiscard]] std::size_t memoryBudgetBytes() const;

    /// Current tracked memory usage in bytes.
    [[nodiscard]] std::size_t currentUsageBytes() const;

    /// Set the memory budget.
    void setBudget(std::size_t bytes);

    /// Track a new allocation (increases current usage).
    void trackAllocation(std::size_t bytes);

    /// Track a deallocation (decreases current usage).
    void trackDeallocation(std::size_t bytes);

    /// Check if a new allocation of the given size would exceed the budget.
    [[nodiscard]] bool wouldExceedBudget(std::size_t bytes) const;

    /// Reset usage tracking (mainly for testing).
    void reset();

    /// The chunk threshold: datasets larger than this use Chunked storage.
    static constexpr std::size_t kChunkThreshold = 100ULL * 1024 * 1024; // 100 MB

private:
    MemoryManager() = default;

    std::size_t budget_ = 4ULL * 1024 * 1024 * 1024; // 4 GB
    std::atomic<std::size_t> currentUsage_{0};
};

} // namespace lumen::data
