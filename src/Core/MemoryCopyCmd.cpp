// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

// TODO: memory

void CommandBuffer::MemoryCopy(const HostToMemoryCopy& desc)
{

}

void CommandBuffer::MemoryCopy(const HostToTextureCopy& desc)
{

}

void CommandBuffer::MemoryCopy(const MemoryToMemoryCopy& copy)
{
    VkBufferCopy bufferCopy = {
        .srcOffset = copy.srcOffset,
        .dstOffset = copy.dstOffset,
        .size = copy.size,
    };

    vkCmdCopyBuffer(
        (VkCommandBuffer)impl.handle,
        impl.device->memories.get(copy.srcMemory).buffer,
        impl.device->memories.get(copy.dstMemory).buffer,
        1, &bufferCopy);
}

void CommandBuffer::MemoryCopy(const MemoryToTextureCopy& copy)
{
    auto& dev = impl.device;
    auto& srcMemory = dev->memories.get(copy.srcMemory);
    auto& dstImg = dev->textures.get(copy.dstTexture);

    VkBufferImageCopy imageCopy = {
        .bufferOffset = copy.srcOffset,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, copy.mipmapLevel, copy.layerBaseLevel, copy.layerCount },
        .imageOffset = { 0,0,0},
        .imageExtent = { dstImg.width, dstImg.height, 1 },
    };

    vkCmdCopyBufferToImage((VkCommandBuffer)impl.handle, srcMemory.buffer, dstImg.image,
        VK_IMAGE_LAYOUT_GENERAL, 1, &imageCopy);
}

void CommandBuffer::MemoryCopy(const TextureToMemory& copy)
{
    auto& dev = impl.device;

    auto& srcImg = dev->textures.get(copy.srcTexture);
    auto& dstMemory = dev->memories.get(copy.dstMemory);

    // I don't wanna this crashing the every moment, so I use this checks/clamps
    auto safeX = std::clamp<int32_t>(copy.x, 0, static_cast<int32_t>(srcImg.width) - 1);
    auto safeY = std::clamp<int32_t>(copy.y, 0, static_cast<int32_t>(srcImg.height) - 1);

    uint32_t maxW = srcImg.width - static_cast<uint32_t>(safeX);
    uint32_t maxH = srcImg.height - static_cast<uint32_t>(safeY);

    auto safeW = std::clamp<uint32_t>(copy.width, 1u, maxW);
    auto safeH = std::clamp<uint32_t>(copy.height, 1u, maxH);

    // std::println("w: {} h: {} x: {} y: {}", safeW, safeH, safeX, safeY);

    VkBufferImageCopy imageCopy = {
        .bufferOffset = copy.dstOffset,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, copy.mipmapLevel, copy.layerBaseLevel, copy.layerCount},
        .imageOffset = {safeX, safeY, 0},
        .imageExtent = {safeW, safeH, 1},
    };

    vkCmdCopyImageToBuffer((VkCommandBuffer)impl.handle, srcImg.image, VK_IMAGE_LAYOUT_GENERAL,
        dstMemory.buffer, 1, &imageCopy);
}

void CommandBuffer::Fill(MemoryView view, uint32_t offset, uint32_t value, uint32_t count)
{
    vkCmdFillBuffer((VkCommandBuffer)impl.handle, impl.device->memories.get(view).buffer, offset, count * sizeof(uint32_t), value);
}