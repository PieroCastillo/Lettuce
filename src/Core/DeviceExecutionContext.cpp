// standard headers
#include <functional>
#include <vector>
#include <mutex>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/DeviceExecutionContext.hpp"

using namespace Lettuce::Core;

inline void record(VkCommandBuffer cmd, VkPipeline& currentPipeline, VkDeviceAddress& currentDescriptorBufferAddress, const Command& command)
{
    if (std::holds_alternative<renderingStartCommand>(command))
    {
        auto renderingCmd = std::get<renderingStartCommand>(command);

        VkRenderingInfo renderingInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {{0,0}, {renderingCmd.width, renderingCmd.height}},
            .layerCount = 1,
            .viewMask = 0,
            .colorAttachmentCount = (uint32_t)renderingCmd.colorAttachments.size(),
            .pColorAttachments = renderingCmd.colorAttachments.data(),
            .pDepthAttachment = renderingCmd.depthAttachment.has_value() ? &(renderingCmd.depthAttachment.value()) : nullptr,
            .pStencilAttachment = renderingCmd.stencilAttachment.has_value() ? &(renderingCmd.stencilAttachment.value()) : nullptr,
        };

        vkCmdBeginRendering(cmd, &renderingInfo);
    }
    else if (std::holds_alternative<renderingEndCommand>(command))
    {
        vkCmdEndRendering(cmd);
    }
    else if (std::holds_alternative<drawCommand>(command))
    {
        auto drawCmd = std::get<drawCommand>(command);

        if (currentPipeline != drawCmd.pipeline)
        {
            currentPipeline = drawCmd.pipeline;
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline);
        }

        if (currentDescriptorBufferAddress != drawCmd.descriptorBufferAddress)
        {
            currentDescriptorBufferAddress = drawCmd.descriptorBufferAddress;
            VkDescriptorBufferBindingInfoEXT  dbbInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                .address = currentDescriptorBufferAddress,
                .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
            };
            vkCmdBindDescriptorBuffersEXT(cmd, 1, &dbbInfo);
        }

        // point to the same buffer (index: 0)
        uint32_t setCount = drawCmd.descriptorBufferOffsets.size();
        std::vector<uint32_t> bufferIndices(setCount, 0);

        vkCmdSetDescriptorBufferOffsetsEXT(cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            drawCmd.pipelineLayout,
            drawCmd.firstSet,
            setCount,
            bufferIndices.data(),
            drawCmd.descriptorBufferOffsets.data()
        );

        if (std::holds_alternative<VkDrawMeshTasksIndirectCommandEXT>(drawCmd.drawArgs))
        {
            auto meshArgs = std::get<VkDrawMeshTasksIndirectCommandEXT>(drawCmd.drawArgs);
            vkCmdDrawMeshTasksEXT(cmd, meshArgs.groupCountX, meshArgs.groupCountY, meshArgs.groupCountZ);
            // command recording finished here
            return;
        }

        // bind vertex/index buffers
        vkCmdBindVertexBuffers(cmd, 0, 1, &(drawCmd.vertexBuffer), &(drawCmd.vertexBufferOffset));

        if (std::holds_alternative<VkDrawIndirectCommand>(drawCmd.drawArgs))
        {
            auto drawArgs = std::get<VkDrawIndirectCommand>(drawCmd.drawArgs);
            vkCmdDraw(cmd, drawArgs.vertexCount, drawArgs.instanceCount, drawArgs.firstVertex, drawArgs.firstInstance);
        }
        else if (std::holds_alternative<VkDrawIndexedIndirectCommand>(drawCmd.drawArgs))
        {
            vkCmdBindIndexBuffer(cmd, drawCmd.indexBuffer, drawCmd.indexBufferOffset, VK_INDEX_TYPE_UINT32);

            auto drawArgs = std::get<VkDrawIndexedIndirectCommand>(drawCmd.drawArgs);
            vkCmdDrawIndexed(cmd, drawArgs.indexCount, drawArgs.instanceCount, drawArgs.firstIndex, drawArgs.vertexOffset, drawArgs.vertexOffset);
        }

    }
    else if (std::holds_alternative<computeCommand>(command))
    {
        auto computeCmd = std::get<computeCommand>(command);
        if (currentPipeline != computeCmd.pipeline)
        {
            currentPipeline = computeCmd.pipeline;
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline);
        }

        if (currentDescriptorBufferAddress != computeCmd.descriptorBufferAddress)
        {
            currentDescriptorBufferAddress = computeCmd.descriptorBufferAddress;
            VkDescriptorBufferBindingInfoEXT  dbbInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                .address = currentDescriptorBufferAddress,
                .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
            };
            vkCmdBindDescriptorBuffersEXT(cmd, 1, &dbbInfo);
        }

        // point to the same buffer (index: 0)
        uint32_t setCount = computeCmd.descriptorBufferOffsets.size();
        std::vector<uint32_t> bufferIndices(setCount, 0);

        vkCmdSetDescriptorBufferOffsetsEXT(cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            computeCmd.pipelineLayout,
            computeCmd.firstSet,
            setCount,
            bufferIndices.data(),
            computeCmd.descriptorBufferOffsets.data()
        );

        vkCmdDispatch(cmd, computeCmd.x, computeCmd.y, computeCmd.z);
    }
}

VkCommandBuffer DeviceExecutionContext::nextCmd()
{
    auto& cmd = m_cmds[m_currentCmdIdx];
    m_currentCmdIdx = (m_currentCmdIdx + 1) % m_cmds.size();
    return  cmd;
}

void DeviceExecutionContext::setupSynchronizationPrimitives(const DeviceExecutionContextCreateInfo& createInfo)
{
    VkSemaphoreTypeCreateInfo semTypeCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = 0,
    };

    VkSemaphoreCreateInfo semCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semTypeCI,
    };

    int semaphoreCount = createInfo.threadCount;
    m_semaphores_g1.reserve(semaphoreCount);
    m_semaphores_g2.reserve(semaphoreCount);

    for(int i = 0; i < semaphoreCount;i++)
    {
        VkSemaphore sem1, sem2;
        vkCreateSemaphore(m_device, &semCI, nullptr, &sem1);
        vkCreateSemaphore(m_device, &semCI, nullptr, &sem2);

        m_semaphores_g1.push_back(sem1);
        m_semaphores_g2.push_back(sem2);
    }
}

void DeviceExecutionContext::setupCommandPools(const DeviceExecutionContextCreateInfo& createInfo)
{
    // lock all cmd pools
    std::vector<std::unique_lock<std::mutex>> locks;
    locks.reserve(m_cmdPoolAccessMutexes.size());

    for (auto& m : m_cmdPoolAccessMutexes)
        locks.emplace_back(m);
    /////////////////////////////

    uint32_t maxDepth = createInfo.maxDepth;

    m_cmdPools.resize(createInfo.threadCount);
    m_cmds.resize(createInfo.threadCount * maxDepth);

    for (uint32_t i = 0; i < createInfo.threadCount; ++i)
    {
        VkCommandPoolCreateInfo cmdPoolCI = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = 0, // TODO: get correct queue family index
        };
        handleResult(vkCreateCommandPool(m_device, &cmdPoolCI, nullptr, &m_cmdPools[i]));

        VkCommandBufferAllocateInfo cmdAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_cmdPools[i],
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = maxDepth,
        };
        handleResult(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_cmds[i * maxDepth]));
    }
}

void DeviceExecutionContext::Create(const IDevice& device, const DeviceExecutionContextCreateInfo& createInfo)
{
    m_device = device.m_device;
    m_graphicsQueue = device.m_graphicsQueue;
    m_computeQueue = device.m_computeQueue;
    m_transferQueue = device.m_transferQueue;

    setupSynchronizationPrimitives(createInfo);
    setupCommandPools(createInfo);
    m_threadPool.Create(createInfo.threadCount);
}

void DeviceExecutionContext::Release()
{
    m_threadPool.Release();

    uint32_t maxDepth = m_cmds.size() / m_cmdPools.size();
    for (int i = 0; i < m_cmdPools.size(); ++i)
    {
        vkFreeCommandBuffers(m_device, m_cmdPools[i], maxDepth, m_cmds.data() + (i * maxDepth));
        vkDestroyCommandPool(m_device, m_cmdPools[i], nullptr);
    }

    // destroy semaphores
    for(int i = 0; i < m_semaphores_g1.size();i++)
    {
        vkDestroySemaphore(m_device, m_semaphores_g1[i], nullptr);
        vkDestroySemaphore(m_device, m_semaphores_g2[i], nullptr);
    }
    m_semaphores_g1.clear();
    m_semaphores_g1.shrink_to_fit();
    m_semaphores_g2.clear();
    m_semaphores_g2.shrink_to_fit();

    m_cmds.clear();
    m_cmds.shrink_to_fit();
    m_cmdPools.clear();
    m_cmdPools.shrink_to_fit();
}

void DeviceExecutionContext::Record(const std::vector<std::tuple<std::vector<Command>, BarriersInfo>>& waves)
{
    // 1 thread
    for (const auto& cmdBuffer : m_cmds)
    {
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        vkBeginCommandBuffer(cmdBuffer, &beginInfo);
    }

    for (auto& [commands, barriers] : waves)
    {
        for (auto& command : commands)
        {
            VkPipeline currentPipeline = VK_NULL_HANDLE;
            VkDeviceAddress currentDescriptorBufferAddress = 0;
            record(nextCmd(), currentPipeline, currentDescriptorBufferAddress, command);
        }

        if (barriers.bufferBarriers.size() != 0 || barriers.imageBarriers.size() != 0)
        {
            VkDependencyInfo dependencyInfo = {
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .bufferMemoryBarrierCount = (uint32_t)barriers.bufferBarriers.size(),
                .pBufferMemoryBarriers = barriers.bufferBarriers.data(),
                .imageMemoryBarrierCount = (uint32_t)barriers.imageBarriers.size(),
                .pImageMemoryBarriers = barriers.imageBarriers.data(),
            };

            for (const auto cmdBuffer : m_cmds)
            {
                vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);
            }
        }
    }

    for (const auto& cmdBuffer : m_cmds)
    {
        vkEndCommandBuffer(cmdBuffer);
    }
}

void DeviceExecutionContext::Execute()
{
    // VkCommandBufferSubmitInfo cmdSubmitInfo = {

    // };

    // VkSubmitInfo2 submitInfo = {
    //     .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
    //     .commandBufferInfoCount = 1,
    // };
    // handleResult(vkResetFences(m_device, 1, &m_fence));
    // handleResult(vkQueueSubmit2(m_graphicsQueue, 1, &submitInfo, m_fence));
}

void DeviceExecutionContext::WaitForExecution()
{
    vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, (std::numeric_limits<uint64_t>::max)());
}