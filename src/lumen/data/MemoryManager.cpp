#include "MemoryManager.h"

namespace lumen::data {

MemoryManager& MemoryManager::instance()
{
    static MemoryManager mgr;
    return mgr;
}

Dataset::StorageMode MemoryManager::decide(std::size_t estimatedBytes) const
{
    if (estimatedBytes > kChunkThreshold) {
        return Dataset::StorageMode::Chunked;
    }
    return Dataset::StorageMode::InMemory;
}

std::size_t MemoryManager::memoryBudgetBytes() const
{
    return budget_;
}

std::size_t MemoryManager::currentUsageBytes() const
{
    return currentUsage_.load(std::memory_order_relaxed);
}

void MemoryManager::setBudget(std::size_t bytes)
{
    budget_ = bytes;
}

void MemoryManager::trackAllocation(std::size_t bytes)
{
    currentUsage_.fetch_add(bytes, std::memory_order_relaxed);
}

void MemoryManager::trackDeallocation(std::size_t bytes)
{
    // Prevent underflow
    std::size_t prev = currentUsage_.load(std::memory_order_relaxed);
    while (true) {
        std::size_t next = (prev >= bytes) ? (prev - bytes) : 0;
        if (currentUsage_.compare_exchange_weak(prev, next, std::memory_order_relaxed)) {
            break;
        }
    }
}

bool MemoryManager::wouldExceedBudget(std::size_t bytes) const
{
    return (currentUsage_.load(std::memory_order_relaxed) + bytes) > budget_;
}

void MemoryManager::reset()
{
    budget_ = 4ULL * 1024 * 1024 * 1024;
    currentUsage_.store(0, std::memory_order_relaxed);
}

} // namespace lumen::data
