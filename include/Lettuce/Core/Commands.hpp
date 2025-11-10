/*
Created by @PieroCastillo on 2025-11-9
*/
#ifndef LETTUCE_CORE_COMMANDS_HPP
#define LETTUCE_CORE_COMMANDS_HPP

// standard headers
#include <vector>
#include <variant>
#include <optional>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    /*
    usually a valid Command Buffer has the next commands:
        CmdBeginCmd()
        CmdBeginRendering(Attachment[])
        BindPipeline(Pipeline, PipelineLayout)
        BindDescriptorBuffers(PipelineLayout, Buffer)
        BindDecriptorBufferOffsets(Offset[])
        SetViewport()
        SetScissor()
        // indirectArgs: buffer,  offset,  drawCount, stride
        // indirectCountArgs: buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride
                |------------------------------ Primitive Shading Pipeline -------------------------------------|-- Mesh Shading Pipeline ----|
                BindVertexBuffers()                                                                             |  ------
                BindIndexBuffer(buffer, offset, idxType)                                                        |  ------
Direct:         Draw(vertexCount, instanceCount, firstVertex,  firstInstance)                                   | DrawMeshTasks(x,y,z)
D Indexed       DrawIndexed(indexCount,  instanceCount,  firstIndex, vertexOffset, firstInstance)               |  -----
Indirect:       DrawIndirect(indirectArgs)                                                                      | DrawMeshTasksIndirect(indirectArgs)
I Indexed:      DrawIndexedIndirect(bindirectArgs)                                        |  -----
IndirectCount:  DrawIndirectCount(indirectCountArgs)         | DrawMeshTasksIndirectCount(indirectCountArgs)
IC Indexed:     DrawIndexedIndirectCount(indirectCountArgs)  |  -----
        CmdEndRendering()
        CmdEndCmd()

    */

    struct renderingStartCommand
    {
        uint32_t width, height;
        std::vector<VkRenderingAttachmentInfo> colorAttachments;
        std::optional<VkRenderingAttachmentInfo> depthAttachment;
        std::optional<VkRenderingAttachmentInfo> stencilAttachment;
    };

    struct renderingEndCommand
    {

    };

    struct drawCommand
    {
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        VkBuffer vertexBuffer;
        VkBuffer indexBuffer;
        uint64_t descriptorBufferAddress;
        uint64_t vertexBufferOffset;
        uint64_t indexBufferOffset;
        std::vector<uint64_t> descriptorBufferOffsets; // set count is embedded in offsets
        uint32_t firstSet;
        std::variant<VkDrawIndirectCommand, VkDrawIndexedIndirectCommand, VkDrawMeshTasksIndirectCommandEXT> drawArgs;
    };

    struct computeCommand
    {
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        uint64_t descriptorBufferAddress;
        std::vector<uint64_t> descriptorBufferOffsets; // set count is embedded in offsets
        uint32_t firstSet;
        uint32_t x, y, z;
    };

    using Command = std::variant<renderingStartCommand, renderingEndCommand, drawCommand, computeCommand>;
    using CommandsList = std::vector<Command>;


    struct BarriersInfo
    {
        std::vector<VkImageMemoryBarrier2> imageBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferBarriers;
    };
}

#endif // LETTUCE_CORE_COMMANDS_HPP