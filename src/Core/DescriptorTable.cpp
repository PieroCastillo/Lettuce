// standard headers
#include <memory>
#include <print>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <ranges>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

DescriptorTable Device::CreateDescriptorTable(const DescriptorTableDesc& desc)
{
    return impl->descriptorTables.allocate({});
}

void Device::Destroy(DescriptorTable descriptorTable)
{
    auto info = impl->descriptorTables.get(descriptorTable);

    impl->descriptorTables.free(descriptorTable);
}

void Device::PushDescriptors(
    DescriptorTable descriptorTable,
    std::span<const std::pair<uint32_t, Sampler>> samplers,
    std::span<const std::pair<uint32_t, Texture>> textures,
    std::span<const std::pair<uint32_t, Allocation>> buffers
)
{

}

void Device::Reset(DescriptorTable descriptorTable)
{

}