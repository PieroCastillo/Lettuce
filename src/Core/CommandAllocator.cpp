// standard headers
#include <array>
#include <limits>
#include <memory>
#include <memory_resource>
#include <print>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <ranges>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

auto Device::CreateCommandAllocator(const CommandAllocatorDesc& desc) -> CommandAllocator
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
    impl->commandAllocators.release(cmdAlloc);
}

void Device::Reset(CommandAllocator cmdAlloc)
{
    auto& cmdPool = impl->commandAllocators.get(cmdAlloc);
    if (cmdPool.cmds.size() > 0) [[likely]]
    {
        vkFreeCommandBuffers(impl->m_device, cmdPool.pool, cmdPool.cmds.size(), cmdPool.cmds.data());
        cmdPool.cmds.clear();
    }
    handleResult(vkResetCommandPool(impl->m_device, impl->commandAllocators.get(cmdAlloc).pool, 0));
}

auto Device::AllocateCommandBuffer(CommandAllocator cmdAlloc) -> CommandBuffer
{
    VkCommandBufferAllocateInfo cmdAI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = impl->commandAllocators.get(cmdAlloc).pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer cmd;
    handleResult(vkAllocateCommandBuffers(impl->m_device, &cmdAI, &cmd));
    // add to cmd list
    impl->commandAllocators.get(cmdAlloc).cmds.push_back(cmd);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    handleResult(vkBeginCommandBuffer(cmd, &beginInfo));

    return CommandBuffer{ CommandBufferImpl{ impl,  (uint64_t)cmd } };
}

void Device::Submit(const CommandBufferSubmitDesc& desc)
{
    WaitFor(SubmitAsync(desc));
}

auto Device::SubmitAsync(const CommandBufferSubmitDesc& desc) -> WaitToken
{
    VkQueue queue = VK_NULL_HANDLE;
    VkSemaphore semaphore = VK_NULL_HANDLE;
    uint64_t currentValue = 0;

    switch (desc.queueType)
    {
    case QueueType::Graphics:
        queue = impl->m_graphicsQueue;
        semaphore = impl->graphicsSemaphore;
        currentValue = impl->graphicsCurrentValue;
        break;

    case QueueType::Compute:
        queue = impl->m_computeQueue;
        semaphore = impl->computeSemaphore;
        currentValue = impl->computeCurrentValue;
        break;

    case QueueType::Copy:
        queue = impl->m_transferQueue;
        semaphore = impl->transferSemaphore;
        currentValue = impl->transferCurrentValue;
        break;
    }

    const uint64_t levelCount = desc.commandBuffers.size();
    const bool hasPresent = desc.presentSwapchain.has_value();

    // Count all command buffers
    uint64_t totalCommandCount = 0;

    for (const auto& level : desc.commandBuffers)
        totalCommandCount += level.size();

    std::byte buffer[4096];
    std::pmr::monotonic_buffer_resource arena(buffer, sizeof(buffer));

    std::pmr::vector<VkSubmitInfo2> submits(&arena);
    std::pmr::vector<VkCommandBufferSubmitInfo> cmdInfos(&arena);
    std::pmr::vector<VkSemaphoreSubmitInfo> semInfos(&arena);
    std::pmr::vector<uint64_t> cmdOffsets(&arena);

    submits.resize(levelCount + (hasPresent ? 1 : 0));
    cmdInfos.resize(totalCommandCount);
    semInfos.resize((levelCount * 2) + (hasPresent ? 2 : 0));
    cmdOffsets.resize(levelCount);

    // Build command buffer information
    uint64_t cmdOffset = 0;

    for (uint64_t i = 0; i < levelCount; ++i)
    {
        cmdOffsets[i] = cmdOffset;

        for (const auto& commandBuffer : desc.commandBuffers[i])
        {
            auto cmd = (VkCommandBuffer)commandBuffer.impl.handle;
            handleResult(vkEndCommandBuffer(cmd));

            cmdInfos[cmdOffset++] = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = cmd,
                .deviceMask = 0,
            };
        }
    }

    // Build submissions, one per level
    for (uint64_t i = 0; i < levelCount; ++i)
    {
        const uint64_t waitValue = currentValue + i;
        const uint64_t signalValue = currentValue + i + 1;

        semInfos[2 * i] = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = semaphore,
            .value = waitValue,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .deviceIndex = 0,
        };

        semInfos[2 * i + 1] = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = semaphore,
            .value = signalValue,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .deviceIndex = 0,
        };

        const uint64_t commandCount = desc.commandBuffers[i].size();

        submits[i] = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &semInfos[2 * i],
            .commandBufferInfoCount = (uint32_t)commandCount,
            .pCommandBufferInfos = commandCount > 0 ? &cmdInfos[cmdOffsets[i]] : nullptr,
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &semInfos[2 * i + 1],
        };
    }

    // Optional presentation submission.
    if (hasPresent)
    {
        const uint64_t submitIdx = levelCount;
        const uint64_t semIdx = levelCount * 2;
        const uint64_t waitValue = currentValue + levelCount;

        auto& swp = impl->swapchains.get(*desc.presentSwapchain);

        semInfos[semIdx] = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = semaphore,
            .value = waitValue,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .deviceIndex = 0,
        };

        semInfos[semIdx + 1] = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore =
                swp.presentSemaphores[(int)swp.currentImageIndex],
            .value = 0,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .deviceIndex = 0,
        };

        submits[submitIdx] = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &semInfos[semIdx],
            .commandBufferInfoCount = 0,
            .pCommandBufferInfos = nullptr,
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &semInfos[semIdx + 1],
        };
    }

    if (!submits.empty())
        handleResult(vkQueueSubmit2(queue, (uint32_t)submits.size(), submits.data(), VK_NULL_HANDLE));

    const uint64_t finalValue = currentValue + levelCount;

    switch (desc.queueType)
    {
    case QueueType::Graphics: impl->graphicsCurrentValue = finalValue;
        break;
    case QueueType::Compute: impl->computeCurrentValue = finalValue;
        break;
    case QueueType::Copy: impl->transferCurrentValue = finalValue;
        break;
    }
    return impl->waitTokens.allocate({ finalValue,desc.queueType });
}