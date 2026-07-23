// standard headers
#include <memory>
#include <array>
#include <print>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

auto Device::SparseBindAsync(QueueType queue, MemoryView mem, std::span<const SparseMemoryBind> binds) -> WaitToken
{
    throw NotImplemented("Sparse Resources are not implemented yet.");
    return impl->waitTokens.allocate({ 0, queue });
}

auto Device::SparseBindAsync(QueueType queue, TextureView tex, std::span<const SparseTextureBind> binds) -> WaitToken
{
    throw NotImplemented("Sparse Resources are not implemented yet.");
    return impl->waitTokens.allocate({ 0, queue });
}