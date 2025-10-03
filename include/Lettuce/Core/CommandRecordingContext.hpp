/*
Created by @PieroCastillo on 2025-07-28
*/
#ifndef LETTUCE_CORE_COMMAND_RECORDING_CONTEXT_HPP
#define LETTUCE_CORE_COMMAND_RECORDING_CONTEXT_HPP

// project headers
#include "common.hpp"
#include "RenderingInfo.hpp"

namespace Lettuce::Core
{
    /*
    vkCmdDraw
    vkCmdDrawIndexed
    vkCmdDrawMeshTasksEXT

    vkCmdDrawIndirect
    vkCmdDrawIndirectCount
    vkCmdDrawIndexedIndirect
    vkCmdDrawIndexedIndirectCount
    vkCmdDrawMeshTasksIndirectEXT
    vkCmdDrawMeshTasksIndirectCountEXT
 */
    struct DrawIndirectArgs
    {
        VkBuffer buffer;
        VkDeviceSize offset;
        uint32_t drawCount;
        uint32_t stride;
        VkBuffer countBuffer;
        VkDeviceSize countBufferOffset;
    };

    class CommandRecordingContext
    {
    private:
    public:
        VkDevice m_device;
        VkCommandBuffer m_cmd;

        CommandRecordingContext(VkDevice device, VkCommandBuffer commandBuffer);
        void Release();

        // Graphics & Compute commands
        void BindRenderingInfo();
        void BindIndexBuffer();
        void BindVertexBuffers();
        void BindPipeline();
        void BindDescriptorTable();

        // Graphics commands
        void ClearColorImage();
        void ClearDepthStencilImage();
        void ClearAttachments();

        void Draw();
        void DrawIndexed();
        void DrawMeshTasks();

        void DrawIndirect(const DrawIndirectArgs& args);


        void BlitImage();
        void ResolveImage();
        /*
            // VK_EXT_vertex_input_dynamic_state
            vkCmdSetVertexInputEXT

            // extended dynamic state commands
            vkCmdSetCullMode
            vkCmdSetDepthBoundsTestEnable
            vkCmdSetDepthCompareOp
            vkCmdSetDepthTestEnable
            vkCmdSetDepthWriteEnable
            vkCmdSetFrontFace
            vkCmdSetPrimitiveTopology
            vkCmdSetScissorWithCount
            vkCmdSetStencilOp
            vkCmdSetStencilTestEnable
            vkCmdSetViewportWithCount

            // extended dynamic state 2 commands
            vkCmdSetDepthBiasEnable
            vkCmdSetLogicOp
            vkCmdSetPatchControlPoints
            vkCmdSetPrimitiveRestartEnable
            vkCmdSetRasterizerDiscardEnable
            // VK_EXT_line_rasterization
            vkCmdSetLineRasterizationModeEXT
            vkCmdSetLineStippleEnableEXT

            // extended dynamic state 3 commands
            vkCmdSetDepthBoundsTestEnableEXT
            vkCmdSetDepthCompareOpEXT
            vkCmdSetDepthTestEnableEXT
            vkCmdSetDepthWriteEnableEXT
            vkCmdSetFrontFaceEXT
            vkCmdSetPrimitiveTopologyEXT
            vkCmdSetScissorWithCountEXT
            vkCmdSetStencilOpEXT
            vkCmdSetStencilTestEnableEXT
            vkCmdSetViewportWithCountEXT
        */
        // Compute commands
        void Dispatch();
        void DispatchIndirect();

        // Transfer commands
        void CopyBuffer();
        void CopyBufferToImage();
        void CopyImageToBuffer();
        void CopyImage();
    };
}
#endif // LETTUCE_CORE_COMMAND_RECORDING_CONTEXT_HPP