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

#include "WorkNode.hpp"
#include "ImageHandle.hpp"
#include "BufferHandle.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Foundation
{
    class WorkFlowGraph
    {
    private:
        uint32_t maxLevel = 0;
        std::vector<std::shared_ptr<WorkNode>> roots;
        std::vector<std::shared_ptr<WorkNode>> nodes;
        void setLevels(std::vector<WorkNode::Edge> &edges);

    public:
        const uint32_t _threads;
        std::shared_ptr<Device> _device;
        // internal objects
        std::vector<VkCommandPool> pools;
        std::vector<VkCommandBuffer> cmds;
        std::vector<std::thread> threads;

        WorkFlowGraph(const std::shared_ptr<Device> &device, uint32_t threads);
        void Release();

        template <CommandData T>
        std::shared_ptr<WorkNode> CreateRootNode(const std::string &name, const std::vector<T> &commands, const std::vector<CommandState> &states)
        {
            auto newRoot = std::make_shared<WorkNode>(name, commands, states);
            roots.push_back(newRoot);
            nodes.push_back(newRoot);
            return newRoot;
        }

        template <CommandData T>
        std::shared_ptr<WorkNode> CreateNode(const std::string &name, const std::vector<T> &commands, const std::vector<CommandState> &states)
        {
            auto node = std::make_shared<WorkNode>(name, commands, states);
            nodes.push_back(node);
            return node;
        }

        BufferHandle AddResource(const std::shared_ptr<BufferResource> &buffer, uint32_t size, uint32_t offset);
        ImageHandle AddResource(const std::shared_ptr<ImageResource> &image, VkImageSubresourceRange range);

        // compile stacks
        void Compile();

        // commands
        void Record(VkCommandBuffer cmd);
    };
}