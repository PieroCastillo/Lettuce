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

#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/ComputePipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Descriptors.hpp"

#include "Lettuce/Foundation/IWorkUnit.hpp"
#include "Lettuce/Foundation/WorkFlowGraph.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Foundation;

// NEEDS REVIEW

WorkFlowGraph::WorkFlowGraph(const std::shared_ptr<Device> &device, uint32_t threads)
    : _device(device), _threads(threads)
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

void WorkFlowGraph::Compile()
{
}

// commands
void WorkFlowGraph::Record(VkCommandBuffer cmd)
{

    vkCmdExecuteCommands(cmd, (uint32_t)cmds.size(), cmds.data());
}
