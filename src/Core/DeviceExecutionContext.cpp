// standard headers
#include <functional>
#include <vector>
#include <mutex>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/DeviceExecutionContext.hpp"

using namespace Lettuce::Core;

void DeviceExecutionContext::setupCommandPools(const DeviceExecutionContextCreateInfo& createInfo)
{
    // lock all cmd pools
    std::vector<std::unique_lock<std::mutex>> locks;
    locks.reserve(cmdPoolAccessMutexes.size());

    for (auto& m : cmdPoolAccessMutexes)
        locks.emplace_back(m);
    /////////////////////////////

    uint32_t cmdCountPerThread = createInfo.maxTasks;

    cmdPools.resize(createInfo.threadCount);
    cmds.resize(createInfo.threadCount * cmdCountPerThread);

    for (uint32_t i = 0; i < createInfo.threadCount; ++i)
    {
        VkCommandPoolCreateInfo cmdPoolCI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = 0, // TODO: get correct queue family index
        };
        handleResult(vkCreateCommandPool(m_device, &cmdPoolCI, nullptr, &cmdPools[i]));

        VkCommandBufferAllocateInfo cmdAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = cmdPools[i],
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = cmdCountPerThread,
        };
        handleResult(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &cmds[i * cmdCountPerThread]));
    }
}

void DeviceExecutionContext::Create(const IDevice& device, const DeviceExecutionContextCreateInfo& createInfo)
{
    m_device = device.m_device;
    setupCommandPools(createInfo);
    threadPool.Create(createInfo.threadCount);
}

void DeviceExecutionContext::Release()
{
    threadPool.Release();

    uint32_t maxTasks = cmds.size() / cmdPools.size();
    for (int i = 0; i < cmdPools.size(); ++i)
    {
        vkFreeCommandBuffers(m_device, cmdPools[i], maxTasks, cmds.data() + (i * maxTasks));
        vkDestroyCommandPool(m_device, cmdPools[i], nullptr);
    }

    cmds.clear();
    cmds.shrink_to_fit();
    cmdPools.clear();
    cmdPools.shrink_to_fit();
}

void DeviceExecutionContext::Prepare(const std::vector<std::vector<CommandList>>& cmds)
{

}

void DeviceExecutionContext::Record()
{
    
}

void DeviceExecutionContext::Execute()
{

}