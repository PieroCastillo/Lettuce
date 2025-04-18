//
// Created by piero on 19/02/2025.
//
#include "Lettuce/Core/Common.hpp"
#include <memory>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/IResource.hpp"
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/ImageResource.hpp"
#include "Lettuce/Core/ResourcePool.hpp"
#include "Lettuce/Core/Semaphore.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/TransferManager.hpp"

using namespace Lettuce::Core;

TransferManager::TransferManager(const std::shared_ptr<Device> &device) : _device(device)
{
    VkCommandPoolCreateInfo poolCI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = _queueFamily,
    };
    checkResult(vkCreateCommandPool(device->GetHandle(), &poolCI, nullptr, &cmdPool));

    VkCommandBufferAllocateInfo cmdAI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    checkResult(vkAllocateCommandBuffers(device->GetHandle(), &cmdAI, &cmd));

    transferFinished = std::make_shared<Semaphore>(device, transferFinishedValue);
}
void TransferManager::Release()
{
    vkFreeCommandBuffers(_device->GetHandle(), cmdPool, 1, &cmd);
    vkDestroyCommandPool(_device->GetHandle(), cmdPool, nullptr);
    transferFinished->Release();
}

void TransferManager::Prepare()
{
    VkCommandBufferBeginInfo cmdBI = {
        .sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    checkResult(vkBeginCommandBuffer(cmd, &cmdBI));
}

void TransferManager::AddTransference(const std::shared_ptr<BufferResource> &src, const std::shared_ptr<ImageResource> &dst, uint32_t mipLevel, TransferType type)
{
    switch (type)
    {
    case TransferType::HostToDevice:
    {
        // copy all layers from a mip level
        VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, mipLevel, 1, 0, dst->_layerCount};
        VkImageMemoryBarrier2 barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
            .srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
            .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .oldLayout = dst->_layout,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = dst->GetHandle(),
            .subresourceRange = range,
        };

        VkDependencyInfo dependency = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        };

        vkCmdPipelineBarrier2(cmd, &dependency);

        VkBufferImageCopy copy = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, mipLevel, 0, dst->_layerCount},
            .imageOffset = {0, 0, 0},
            .imageExtent = {dst->_width, dst->_height, dst->_depth},
        };

        vkCmdCopyBufferToImage(cmd, src->GetHandle(), dst->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

        barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
        barrier.dstAccessMask = VK_ACCESS_2_NONE;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        dst->_layout = VK_IMAGE_LAYOUT_GENERAL;
        vkCmdPipelineBarrier2(cmd, &dependency);
    }
    break;

    default:
        break;
    }
}

void TransferManager::AddTransference(const std::shared_ptr<BufferResource> &src, const std::shared_ptr<BufferResource> &dst, TransferType type)
{
    if (src->_size != dst->_size)
    {
        throw std::runtime_error("sizes of srd and dst buffers must be equal.");
        return;
    }

    switch (type)
    {
    case TransferType::HostToDevice:
    {
        VkBufferCopy copy = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = src->_size,
        };

        vkCmdCopyBuffer(cmd, src->GetHandle(), dst->GetHandle(), 1, &copy);
    }
    break;

    default:
        break;
    }
}

void TransferManager::AddTransference(VkImageSubresourceLayers srcSubresource, VkImageSubresourceLayers dstSubresource, const std::shared_ptr<ImageResource> &src, const std::shared_ptr<ImageResource> &dst, TransferType type)
{
    // if (src->_size != dst->_size)
    // {
    //     throw std::runtime_error("sizes of srd and dst images must be equal.");
    //     return;
    // }

    switch (type)
    {
    case TransferType::HostToDevice:
    {
        VkImageCopy copy = {
            .srcSubresource = srcSubresource,
            .srcOffset = {0, 0, 0},
            .dstSubresource = dstSubresource,
            .dstOffset = {0, 0, 0},
            .extent = {src->_width, src->_height, src->_depth},
        };

        vkCmdCopyImage(cmd, src->GetHandle(), src->_layout, dst->GetHandle(), dst->_layout, 1, &copy);
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
        .semaphore = transferFinished->GetHandle(),
        .value = transferFinishedValue + 1,
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
    checkResult(vkQueueSubmit2(_device->_graphicsQueues[0], 1, &submit2I, VK_NULL_HANDLE));
    transferFinished->Wait(transferFinishedValue + 1);
    transferFinishedValue++;
}
