//
// Created by piero on 28/02/2025.
//
#include <iostream>
#include <string>
#include <functional>
#include <tuple>
#include <algorithm>
#include <memory>
#include <thread>
#include <vector>
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/ImageResource.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Semaphore.hpp"
#include "Lettuce/Core/RenderPass.hpp"

#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/ComputePipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Descriptors.hpp"

#include "Lettuce/Foundation/IWorkUnit.hpp"
#include "Lettuce/Foundation/WorkFlowGraph.hpp"

using namespace Lettuce::Core;

using namespace Lettuce::Foundation;

WorkFlowGraph::WorkFlowGraph(const std::shared_ptr<Device> &device, const std::shared_ptr<RenderPass> &renderPass, uint32_t threads)
    : _device(device), _renderPass(renderPass), _threads(threads)
{
    pools = std::vector<VkCommandPool>(threads, VK_NULL_HANDLE);
    cmds = std::vector<VkCommandBuffer>(threads, VK_NULL_HANDLE);

    VkCommandPoolCreateInfo poolCI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = device->_gpu.graphicsFamily.value(),
    };

    VkCommandBufferAllocateInfo cmdAI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        //.commandPool
        .level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = 1,
    };

    for (int i = 0; i < threads; i++)
    {
        vkCreateCommandPool(device->_device, &poolCI, nullptr, &(pools[i]));
        cmdAI.commandPool = pools[i];
        vkAllocateCommandBuffers(device->_device, nullptr, &(cmds[i]));
    }
}

void WorkFlowGraph::Release()
{
    for (int i = 0; i < _threads; i++)
    {
        vkFreeCommandBuffers(_device->_device, pools[i], 1, &cmds[i]);
        vkDestroyCommandPool(_device->_device, pools[i], nullptr);
    }
}

// add resources
void WorkFlowGraph::AddBuffer(const std::shared_ptr<BufferResource> &buffer, uint32_t offset, uint32_t size)
{
    if (!canAddResources)
        return;
}
void WorkFlowGraph::AddImage(const std::shared_ptr<ImageResource> &image)
{
    if (!canAddResources)
        return;
}
void WorkFlowGraph::AddWorkUnit(const IWorkUnit &workUnit)
{
    if (!canAddResources)
        return;
}

void WorkFlowGraph::StopAddResources()
{
    canAddResources = false;
}

// add nodes
void WorkFlowGraph::WorkNode(int nodeSrc, int nodeDst, VkPipelineStageFlags2 srcMask, VkPipelineStageFlags2 dstMask)
{
    edges[nodeSrc].push_back({nodeDst, srcMask, dstMask});
}
void WorkFlowGraph::UseBuffer(int node, int bufferIndex, ResourceUsage usage)
{
}
void WorkFlowGraph::UseImage(int node, int imageIndex, ResourceUsage usage)
{
}

void WorkFlowGraph::Compile()
{
    int componentsPerStack = workUnits.size() / _threads;
    std::vector<bool> visited(workUnits.size(), false);
    std::vector<int> workUnitSorted(workUnits.size(), 0);

    // topological sort
    for(int i = 0; i < workUnits.size();i++)
    {
        if(visited[i])
            continue;

        
    }
}

// commands
void WorkFlowGraph::StartRecording(VkCommandBuffer cmd)
{
}
void WorkFlowGraph::Record()
{
}
void WorkFlowGraph::EndRecording(VkCommandBuffer cmd)
{
}
