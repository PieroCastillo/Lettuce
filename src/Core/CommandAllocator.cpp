// standard headers
#include <array>
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

CommandAllocator Device::CreateCommandAllocator(const CommandAllocatorDesc& desc)
{
    VkCommandPoolCreateInfo poolCI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    };
    switch (desc.queueType)
    {
    case QueueType::Graphics: poolCI.queueFamilyIndex = impl->props.graphicsQueueFamilyIdx; break;
    case QueueType::Compute: poolCI.queueFamilyIndex = impl->props.computeQueueFamilyIdx; break;
    case QueueType::Copy: poolCI.queueFamilyIndex = impl->props.transferQueueFamilyIdx; break;
    }
    VkCommandPool pool;
    handleResult(vkCreateCommandPool(impl->m_device, &poolCI, nullptr, &pool));

    return impl->commandAllocators.allocate({ pool });
}

void Device::Destroy(CommandAllocator cmdAlloc)
{
    auto poolInfo = impl->commandAllocators.get(cmdAlloc);
    handleResult(vkResetCommandPool(impl->m_device, poolInfo.pool, 1));
    vkDestroyCommandPool(impl->m_device, poolInfo.pool, nullptr);
    impl->commandAllocators.free(cmdAlloc);
}

void Device::Reset(CommandAllocator cmdAlloc)
{
    handleResult(vkResetCommandPool(impl->m_device, impl->commandAllocators.get(cmdAlloc).pool, 1));
}

CommandBuffer Device::AllocateCommandBuffer(CommandAllocator cmdAlloc)
{
    VkCommandBufferAllocateInfo cmdAI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = impl->commandAllocators.get(cmdAlloc).pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer cmd;
    handleResult(vkAllocateCommandBuffers(impl->m_device, &cmdAI, &cmd));

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    handleResult(vkBeginCommandBuffer(cmd, &beginInfo));

    return CommandBuffer{ CommandBufferImpl{ impl,  (uint64_t)cmd } };
}


void Device::Submit(const CommandBufferSubmitDesc& desc)
{

}