// standard headers
#include <array>
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
    VkQueue queue;
    VkSemaphore semaphore;
    uint64_t currentValue;

    switch (desc.queueType)
    {
    case QueueType::Graphics:
        queue = impl->m_graphicsQueue;
        semaphore = impl->graphicsSemaphore;
        currentValue = impl->graphicsCurrentValue; break;
    case QueueType::Compute:
        queue = impl->m_computeQueue;
        semaphore = impl->computeSemaphore;
        currentValue = impl->computeCurrentValue; break;
    case QueueType::Copy:
        queue = impl->m_transferQueue;
        semaphore = impl->transferSemaphore;
        currentValue = impl->transferCurrentValue; break;
    }

    constexpr uint32_t maxCmdLevels = 16;
    constexpr uint32_t maxCmdCountPerLevel = 16;
    std::byte buffer[4096];
    std::pmr::monotonic_buffer_resource arena(buffer, sizeof(buffer));

    std::pmr::vector<VkSubmitInfo2> submits(&arena);
    submits.resize(desc.commandBuffers.size() + (desc.presentSwapchain ? 1 : 0));
    std::pmr::vector<VkCommandBufferSubmitInfo> cmdInfos(&arena);
    cmdInfos.resize(maxCmdLevels * maxCmdCountPerLevel);
    std::pmr::vector<VkSemaphoreSubmitInfo> semInfos(&arena);
    semInfos.resize((2 * desc.commandBuffers.size()) + (desc.presentSwapchain ? 2 : 0));

    // per level
    auto waitValue = currentValue;
    auto signalValue = currentValue + 1;
    for (int i = 0; i < desc.commandBuffers.size(); ++i)
    {
        // per cmd
        for (int j = 0; j < desc.commandBuffers[i].size(); ++j)
        {
            auto cmd = (VkCommandBuffer)(desc.commandBuffers[i][j].impl.handle);
            handleResult(vkEndCommandBuffer(cmd));

            cmdInfos[(i * maxCmdLevels) + j] = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = cmd,
                .deviceMask = 0,
            };
        }
        // wait
        semInfos[2 * i] = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = semaphore,
            .value = waitValue,
            .stageMask = VK_PIPELINE_STAGE_NONE,
            .deviceIndex = 0,
        };
        // signal
        semInfos[2 * i + 1] = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = semaphore,
            .value = signalValue,
            .stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            .deviceIndex = 0,
        };

        submits[i] = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &semInfos[2 * i],
            .commandBufferInfoCount = (uint32_t)(desc.commandBuffers[i].size()),
            .pCommandBufferInfos = &cmdInfos[i * maxCmdLevels],
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &semInfos[2 * i + 1],
        };
        ++waitValue;
        ++signalValue;
    }

    if (desc.presentSwapchain)
    {
        // wait timeline semaphore
        semInfos[semInfos.size() - 2] = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = semaphore,
            .value = waitValue,
            .stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .deviceIndex = 0,
        };
        // signal binary semaphore
        auto& swp = impl->swapchains.get(*desc.presentSwapchain);
        semInfos[semInfos.size() - 1] = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = swp.presentSemaphores[(int)(swp.currentImageIndex)],
            .stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            .deviceIndex = 0,
        };

        submits[submits.size() - 1] = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &semInfos[semInfos.size() - 2],
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &semInfos[semInfos.size() - 1],
        };
    }

    handleResult(vkQueueSubmit2(queue, (uint32_t)submits.size(), submits.data(), VK_NULL_HANDLE));

    switch (desc.queueType)
    {
    case QueueType::Graphics: impl->graphicsCurrentValue = signalValue - 1; break;
    case QueueType::Compute: impl->computeCurrentValue = signalValue - 1; break;
    case QueueType::Copy: impl->transferCurrentValue = signalValue - 1; break;
    }
}