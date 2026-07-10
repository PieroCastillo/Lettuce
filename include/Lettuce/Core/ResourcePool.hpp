#ifndef LETTUCE_CORE_RESOURCE_POOL
#define LETTUCE_CORE_RESOURCE_POOL

// standard headers
#include <atomic>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

// helpers
#include "../helper.hpp"

namespace Lettuce::Core
{
    template<typename HandleT, typename ObjectT, bool MultiThread = false>
    class ResourcePool
    {
    private:
        struct NullLock { constexpr explicit NullLock(std::mutex&) noexcept {} };
        using PoolLock = std::conditional_t<MultiThread, std::lock_guard<std::mutex>, NullLock>;
    public:
        HandleT allocate(ObjectT&& obj) {
            PoolLock lock(mutex);

            if (!freeList.empty()) {
                uint32_t idx = freeList.back();
                freeList.pop_back();

                objects[idx] = std::move(obj);
                validCount.fetch_add(1, std::memory_order_relaxed);

                return HandleT{ idx, generations[idx] };
            }

            uint32_t idx = static_cast<uint32_t>(objects.size());
            objects.push_back(std::move(obj));
            generations.push_back(1);

            validCount.fetch_add(1, std::memory_order_relaxed);

            return HandleT{ idx, 1 };
        }

        void release(HandleT h) {
            PoolLock lock(mutex);

            if (!isValidUnlocked(h)) return;

            generations[h.index]++;
            freeList.push_back(h.index);
            validCount.fetch_sub(1, std::memory_order_relaxed);
        }

        ObjectT& get(HandleT h) {
            PoolLock lock(mutex);

            DebugAssert(isValidUnlocked(h), "[RESOURCE POOL] invalid resource access.");
            return objects[h.index];
        }

        bool isValid(HandleT h) const {
            PoolLock lock(mutex);

            return isValidUnlocked(h);
        }

        uint32_t getHandleCount() const noexcept {
            return validCount.load(std::memory_order_relaxed);
        }

    private:
        bool isValidUnlocked(HandleT h) const {
            return h.index < generations.size() && generations[h.index] == h.generation;
        }

    private:
        std::vector<ObjectT> objects;
        std::vector<uint32_t> generations;
        std::vector<uint32_t> freeList;

        std::atomic<uint32_t> validCount{ 0 };
        mutable std::mutex mutex;
    };
}
#endif // LETTUCE_CORE_RESOURCE_POOL