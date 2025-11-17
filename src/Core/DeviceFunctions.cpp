// standard headers
#include <array>
#include <expected>
#include <iostream>
#include <memory>
#include <print>
#include <variant>
#include <vector>

// project headers
#include "Lettuce/Core/Device.hpp"

// external headers
#include <ktx.h>
#define VK_NO_PROTOTYPE
#include <ktxvulkan.h>

using namespace Lettuce::Core;

auto Device::MemoryCopy(const DeviceVectorBase& src, const DeviceVectorBase& dst, uint32_t srcIdx, uint32_t dstIdx, uint32_t count) -> void
{
    if (src.typeIdx != dst.typeIdx) [[unlikely]]
    {
        throw LettuceException(LettuceResult::TypeMismatch);
    }
    uint32_t elementSize = src.m_elementSize;

    // bounds checking
    if (!(srcIdx * elementSize < src.m_maxSize && dstIdx * elementSize < dst.m_maxSize)) [[unlikely]]
    {
        throw LettuceException(LettuceResult::OutOfBounds);
    }

    VkBufferCopy copy = {
        .srcOffset = srcIdx * elementSize,
        .dstOffset = dstIdx * elementSize,
        .size = count * elementSize,
    };
    m_copies.emplace_back(src.m_allocation.buffer, dst.m_allocation.buffer, std::move(copy));
}


auto Device::FlushCopies() -> void
{
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(m_copyCmd, &beginInfo);

    for (auto [src, dst, copy] : m_copies) {
        vkCmdCopyBuffer(m_copyCmd, src, dst, 1, &copy);
    }
    
    vkEndCommandBuffer(m_copyCmd);

    vkResetFences(m_device, 1, &m_copyFence);
    constexpr auto timeout = (std::numeric_limits<uint32_t>::max)();

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_copyCmd,
    };

    handleResult(vkQueueSubmit(m_transferQueue, 1, &submitInfo, m_copyFence));
    handleResult(vkWaitForFences(m_device, 1, &m_copyFence, VK_TRUE, timeout));
}