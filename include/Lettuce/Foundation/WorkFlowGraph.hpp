//
// Created by piero on 28/02/2025.
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
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Semaphore.hpp"

#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/ComputePipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Descriptors.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Foundation
{
    enum class ResourceUsage
    {
        Read,
        Write,
        ReadWrite,
    };
    struct BufferAsResourceInfo
    {
        std::shared_ptr<BufferResource> bufferPtr;
        VkPipelineStageFlagBits2 stageBit;
        ResourceUsage usage;
    };

    struct ImageAsResourceInfo
    {
        std::shared_ptr<ImageResource> imgPtr;
        VkPipelineStageFlagBits2 stageBit;
        VkImageLayout layout;
        ResourceUsage usage;
    };

    struct ImageViewAsResourceInfo
    {
        std::shared_ptr<ImageViewResource> imgVwPtr;
        VkPipelineStageFlagBits2 stageBit;
        VkImageLayout layout;
        ResourceUsage usage;
    };

    /// @brief Organizes Work to be submmitted to GPU
    ///
    /// @brief has 3 stages:
    ///
    /// 1. Add Stuff (WorkNodes and Resources)
    ///
    /// 2. Add Nodes
    ///
    /// 3. Compile
    class WorkFlowGraph
    {
    private:
        bool canAddResources = true;

        std::vector<std::vector<std::function<void(VkCommandBuffer)>>> funcs;

    public:
        const uint32_t _threads;
        std::shared_ptr<Device> _device;
        // internal objects
        std::vector<VkCommandPool> pools;
        std::vector<VkCommandBuffer> cmds;
        std::vector<std::thread> threads;

        WorkFlowGraph(const std::shared_ptr<Device> &device, uint32_t threads);
        void Release();

        // std::shared_ptr<WorkNode> CreateNode();
        // std::shared_ptr<ExecutableWorkNode> CreateNode();

        // compile stacks
        void Compile();

        // commands
        void Record(VkCommandBuffer cmd);
    };
}