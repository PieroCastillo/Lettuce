/*
Created by @PieroCastillo on 2025-07-28
*/
#ifndef LETTUCE_CORE_COMMAND_LIST_HPP
#define LETTUCE_CORE_COMMAND_LIST_HPP

namespace Lettuce::Core
{
    class CommandList
    {
    private:
    public:
        VkDevice m_device;
        VkCommandBuffer m_cmd;

        CommandList(VkDevice device, VkCommandBuffer commandBuffer);
        void Release();

        // Graphics & Compute commands
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
        void DrawIndirect();
        void DrawIndexedIndirect();

        void DrawMeshTasks();
        void DrawMeshTasksIndirect();

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
#endif // LETTUCE_CORE_COMMAND_LIST_HPP