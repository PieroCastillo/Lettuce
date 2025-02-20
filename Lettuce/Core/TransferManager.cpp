//
// Created by piero on 19/02/2025.
//
#pragma once
#include "Lettuce/Core/common.hpp"
#include <memory>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/TransferManager.hpp"
#include "Lettuce/Core/Semaphore.hpp"
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

TransferManager::TransferManager(const std::shared_ptr<Device> &device) : _device(device)
{
    VkCommandPoolCreateInfo poolCI = {
        sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        queueFamilyIndex = _queueFamily,
    };
    checkResult(vkCreateCommandPool(device->_device, &poolCI, nullptr, &cmdPool));

    VkCommandBufferAllocateInfo cmdAI = {
        sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        commandPool = cmdPool,
        level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        commandBufferCount = 1,
    };
    checkResult(vkAllocateCommandBuffers(device->_device, &cmdAI, &cmd));
}
TransferManager::~TransferManager()
{
    vkFreeCommandBuffers(device->_device, cmdPool, 1, &cmd);
    vkDestroyCommandPool(device->_device, cmdPool, nullptr);
}

void TransferManager::Prepare()
{
    VkCommandBufferBeginInfo cmdBI = {
        sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    checkResult(vkBeginCommandBuffer(cmd, &cmdBI));
}
void TransferManager::AddTransference(const std::shared_ptr<BufferResource> &src, const std::shared_ptr<BufferResource> &dst, TransferType type)
{
    if (src->_size != dst->_size)
    {
        throw std::runtime_error(std::format("sizes of srd and dst buffers must be equal."));
        return;
    }

    switch (type)
    {
    case TransferType::HostToDevice:
    {
        VkBufferCopy copy = {
            srcOffset = 0,
            dstOffset = 0,
            size = src->_size,
        };

        vkCmdCopyBuffer(cmd, src->_buffer, dst->_buffer, 1, &copy);
    }
    break;

    default:
        break;
    }
}

void TransferManager::TransferAll()
{
    checkResult(vkEndCommandBuffer(cmd));

    // here we start the transfer process

    VkCommandBufferSubmitInfo cmdSI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
        .deviceMask = 0,
    };

    VkSemaphoreSubmitInfo signalSI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = transferFinished->_semaphore,
        .value = renderFinishedValue + 1,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .deviceIndex = 0,
    };

    VkSubmitInfo2 submit2I = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdSI,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signalSI,
    };
    checkResult(vkQueueSubmit2(device->_graphicsQueues[0], 1, &submit2I, VK_NULL_HANDLE));
    transferFinished->Wait(transferFinishedValue + 1);
    transferFinishedValue++;
}
