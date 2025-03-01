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
#include "IWorkUnit.hpp"
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/ImageResource.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Semaphore.hpp"
#include "Lettuce/Core/RenderPass.hpp"

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

    class WorkFlowGraph
    {
    private:
    public:
        const uint32_t _threads;
        std::shared_ptr<Device> _device;
        std::shared_ptr<RenderPass> _renderPass;
        std::vector<VkCommandPool> pools;
        std::vector<VkCommandBuffer> cmds;
        std::vector<std::thread> threads;

        WorkFlowGraph(const std::shared_ptr<Device> &device, const std::shared_ptr<RenderPass> &renderPass, uint32_t threads);
        ~WorkFlowGraph();

        // add resources
        void AddBuffer(const std::shared_ptr<BufferResource> &buffer, uint32_t offset, uint32_t size);
        void AddImage(const std::shared_ptr<ImageResource> &image);
        void AddWorkUnit(const IWorkUnit &workUnit);

        // add nodes
        void WorkNode(int nodeSrc, int nodeDst, VkPipelineStageFlags2 srcMask, VkPipelineStageFlags2 dstMask);
        void UseBuffer(int node, int bufferIndex, ResourceUsage usage);
        void UseImage(int node, int imageIndex, ResourceUsage usage);

        // commands
        void StartRecording(VkCommandBuffer cmd);
        void Record();
        void EndRecording(VkCommandBuffer cmd);
    };
}