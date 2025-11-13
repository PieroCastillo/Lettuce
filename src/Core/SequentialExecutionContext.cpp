// project headers
#include "Lettuce/Core/SequentialExecutionContext.hpp"

using namespace Lettuce::Core;

void SequentialExecutionContext::record(VkCommandBuffer cmd, VkPipeline& currentPipeline, VkDeviceAddress& currentDescriptorBufferAddress, const Command& command)
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

        if (drawCmd.descriptorBufferAddress != 0 && currentDescriptorBufferAddress != drawCmd.descriptorBufferAddress)
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
        if (drawCmd.descriptorBufferAddress != 0 && drawCmd.descriptorBufferOffsets.size() > 0)
        {
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
        }

        if (std::holds_alternative<VkDrawMeshTasksIndirectCommandEXT>(drawCmd.drawArgs))
        {
            auto meshArgs = std::get<VkDrawMeshTasksIndirectCommandEXT>(drawCmd.drawArgs);
            vkCmdDrawMeshTasksEXT(cmd, meshArgs.groupCountX, meshArgs.groupCountY, meshArgs.groupCountZ);
            // command recording finished here
            return;
        }

        // bind vertex buffers
        if (drawCmd.vertexBuffers.size() > 0) {
            vkCmdBindVertexBuffers(cmd, 0, (uint32_t)drawCmd.vertexBuffers.size(), drawCmd.vertexBuffers.data(), drawCmd.vertexOffsets.data());
        }

        VkViewport viewport = {
            0, 0, drawCmd.width, drawCmd.height, 0, 1
        };
        VkRect2D scissor = {
            {0, 0}, {drawCmd.width, drawCmd.height}
        };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        if (std::holds_alternative<VkDrawIndirectCommand>(drawCmd.drawArgs))
        {
            auto drawArgs = std::get<VkDrawIndirectCommand>(drawCmd.drawArgs);
            vkCmdDraw(cmd, drawArgs.vertexCount, drawArgs.instanceCount, drawArgs.firstVertex, drawArgs.firstInstance);
        }
        else if (std::holds_alternative<VkDrawIndexedIndirectCommand>(drawCmd.drawArgs))
        {
            if (drawCmd.indexBuffer != VK_NULL_HANDLE)
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
    else if (std::holds_alternative<renderingEndCommand>(command))
    {
        vkCmdEndRendering(cmd);
    }
}

void SequentialExecutionContext::Create(const IDevice& device, const SequentialExecutionContextCreateInfo& createInfo)
{
    m_device = device.m_device;
    m_queue = device.m_graphicsQueue;
    m_currentWaitValue = 0;

    VkCommandPoolCreateInfo cmdPoolCI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = 0,
    };
    handleResult(vkCreateCommandPool(m_device, &cmdPoolCI, nullptr, &m_cmdPool));

    VkCommandBufferAllocateInfo cmdAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_cmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    handleResult(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_cmd));

    VkSemaphoreTypeCreateInfo semTypeCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = 0,
    };

    VkSemaphoreCreateInfo semCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semTypeCI,
    };
    handleResult(vkCreateSemaphore(m_device, &semCI, nullptr, &m_renderFinished));
}

void SequentialExecutionContext::Release()
{
    vkDestroySemaphore(m_device, m_renderFinished, nullptr);
    vkFreeCommandBuffers(m_device, m_cmdPool, 1, &m_cmd);
    vkDestroyCommandPool(m_device, m_cmdPool, nullptr);
}

CommandRecordingContext& SequentialExecutionContext::GetCommandList()
{
    return m_cmdContext;
}

void SequentialExecutionContext::Execute()
{
    ++m_currentWaitValue;

    VkCommandBufferBeginInfo cmdBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(m_cmd, &cmdBeginInfo);

    VkPipeline currentPipeline = VK_NULL_HANDLE;
    VkDeviceAddress currentDescriptorBufferAddress = 0;

    auto commands = m_cmdContext.GetCommands();
    for (auto& command : commands)
    {
        record(m_cmd, currentPipeline, currentDescriptorBufferAddress, command);
    }

    vkEndCommandBuffer(m_cmd);

    VkCommandBufferSubmitInfo cmdSubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = m_cmd,
        .deviceMask = 0,
    };

    VkSemaphoreSubmitInfo semWaitInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_renderFinished,
        .value = (m_currentWaitValue - 1),
        .stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
        .deviceIndex = 0,
    };

    VkSemaphoreSubmitInfo semSignalInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_renderFinished,
        .value = m_currentWaitValue,
        .stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
        .deviceIndex = 0,
    };

    VkSubmitInfo2 submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        // .waitSemaphoreInfoCount = 1,
        // .pWaitSemaphoreInfos= &semWaitInfo,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdSubmitInfo,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &semSignalInfo,
    };
    handleResult(vkQueueSubmit2(m_queue, 1, &submitInfo, VK_NULL_HANDLE));
}

void SequentialExecutionContext::Wait()
{
    VkSemaphoreWaitInfo waitInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = 1,
        .pSemaphores = &m_renderFinished,
        .pValues = &m_currentWaitValue,
    };
    handleResult(vkWaitSemaphores(m_device, &waitInfo, (std::numeric_limits<uint64_t>::max)()));
}