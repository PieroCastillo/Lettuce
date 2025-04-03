//
// Created by piero on 1/04/2025.
//
#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <tuple>
#include <algorithm>
#include <memory>
#include <thread>
#include <vector>
#include <variant>
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/ImageResource.hpp"
#include "Lettuce/Core/ImageViewResource.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Semaphore.hpp"

#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/ComputePipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Descriptors.hpp"

#include "RenderTargetView.hpp"
#include "RenderTaskGroup.hpp"
#include "RenderTask.hpp"
#include "ImageHandle.hpp"
#include "BufferHandle.hpp"
#include "CommandList.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Foundation
{
    class RenderDispatcher
    {
    private:
        const uint32_t _threads;
        std::shared_ptr<Device> _device;
        // internal objects
        std::vector<VkCommandPool> pools;
        std::vector<VkCommandBuffer> cmds;
        std::vector<std::thread> threads;

    public:
        RenderDispatcher(const std::shared_ptr<Device> &device, uint32_t threads);
        void Release();

        // add resource methods
        RenderTargetView AddRenderTargetView(RenderTargetView&& renderTargetView);
        BufferHandle AddResource(BufferHandle&& buffer);
        ImageHandle AddResource(ImageHandle&& image);

        RenderTaskGroup CreateGroup(std::string &groupName, VkPipelineBindPoint bindPoint);

        void Barrier(RenderTaskGroup &srcGroup, RenderTaskGroup &dstGroup,
                     BufferHandle buffer,
                     VkPipelineStageFlags2 srcStage,
                     VkPipelineStageFlags2 dstStage,
                     VkAccessFlags2 srcAccess,
                     VkAccessFlags2 dstAccess);

        void Barrier(RenderTaskGroup &srcGroup, RenderTaskGroup &dstGroup,
                     ImageHandle image,
                     VkImageLayout layout,
                     VkPipelineStageFlags2 srcStage,
                     VkPipelineStageFlags2 dstStage,
                     VkAccessFlags2 srcAccess,
                     VkAccessFlags2 dstAccess);

        // compile stacks
        void Preprocess();

        // commands
        void Record(VkCommandBuffer cmd);
    };
}