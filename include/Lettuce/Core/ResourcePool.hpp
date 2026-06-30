#ifndef LETTUCE_CORE_RESOURCE_POOL
#define LETTUCE_CORE_RESOURCE_POOL

// standard headers
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

namespace Lettuce::Core
{
    template<typename HandleT, typename ObjectT>
    class ResourcePool {
    public:
        HandleT allocate(ObjectT&& obj) {
            if (!freeList.empty()) {
                uint32_t idx = freeList.back();
                freeList.pop_back();

                objects[idx] = std::move(obj);

                return HandleT{ idx, generations[idx] };
            }

            uint32_t idx = static_cast<uint32_t>(objects.size());
            objects.push_back(std::move(obj));
            generations.push_back(1);

            return HandleT{ idx, 1 };
        }

        void release(HandleT h) {
            if (!isValid(h)) return;

            generations[h.index]++;
            freeList.push_back(h.index);
        }

        ObjectT& get(HandleT h) {
            assert(isValid(h));
            return objects[h.index];
        }

        bool isValid(HandleT h) const {
            return h.index < generations.size() && generations[h.index] == h.generation;
        }

    private:
        std::vector<ObjectT> objects;
        std::vector<uint32_t> generations;
        std::vector<uint32_t> freeList;
    };
}
#endif // LETTUCE_CORE_RESOURCE_POOL