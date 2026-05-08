// standard headers
#include <array>
#include <memory>
#include <print>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <ranges>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

// TODO: memory

auto Device::CreateMemoryView(const MemoryViewDesc& desc)
{
    auto mem = impl->selectAlloc(desc.policy, desc.cpuVisible)->AllocateMemory(desc.size);
    // TODO: error check logic
    return mem;
}

auto Device::Destroy(MemoryView view)
{
    /*
    auto info = impl->buffers.get(buffer);

    vkDestroyBuffer(impl->m_device, impl->buffers.get(buffer).buffer, nullptr);
    impl->buffers.free(buffer);
    */
}