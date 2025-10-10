/*
Created by @PieroCastillo on 2025-07-28
*/
#ifndef LETTUCE_CORE_COMMAND_RECORDING_CONTEXT_HPP
#define LETTUCE_CORE_COMMAND_RECORDING_CONTEXT_HPP

// standard headers
#include <vector>

// project headers
#include "common.hpp"
#include "DescriptorTable.hpp"
#include "Pipeline.hpp"
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

    struct drawCommand {
        enum Type { Dispatch, DrawIndexed, DrawIndirect, BindPipeline, BindDescriptor, BindVertexBuffer, BindIndexBuffer } type;
        union {
            struct { uint32_t x, y, z;} dispatch;
            struct { uint32_t indexCount, instanceCount, firstIndex, vertexOffset, firstInstance; } drawIndexed;
            struct { VkBuffer buffer; VkDeviceSize offset; uint32_t drawCount; uint32_t stride; } drawIndirect;
            struct { VkPipeline pipeline; } bindPipeline;
            struct { VkBuffer set; } bindDescriptorTable;
        };
    };

    struct computeCommand {
        enum Type { Dispatch, BindPipeline, BindDescriptor } type;
        union {
            struct { uint32_t x, y, z;} dispatch;
            struct { VkPipeline pipeline; } bindPipeline;
            struct { VkBuffer set; } bindDescriptorTable;
        };
    };

    struct transferCommand {
        enum Type { CopyBuffer, CopyBufferToImage, CopyImageToBuffer, CopyImage } type;
        union {
            struct { VkBuffer srcBuffer; VkBuffer dstBuffer; VkDeviceSize size; VkDeviceSize srcOffset; VkDeviceSize dstOffset; } copyBuffer;
            struct { VkBuffer srcBuffer; VkImage dstImage; uint32_t width; uint32_t height; uint32_t depth; VkDeviceSize bufferOffset; uint32_t mipLevel; uint32_t arrayLayer; } copyBufferToImage;
            struct { VkImage srcImage; VkBuffer dstBuffer; uint32_t width; uint32_t height; uint32_t depth; VkDeviceSize bufferOffset; uint32_t mipLevel; uint32_t arrayLayer; } copyImageToBuffer;
            struct { VkImage srcImage; VkImage dstImage; uint32_t width; uint32_t height; uint32_t depth; uint32_t srcMipLevel; uint32_t srcArrayLayer; uint32_t dstMipLevel; uint32_t dstArrayLayer; } copyImage;
        };
    };

    struct DrawIndexedArgs
    {

    };

    struct DrawIndirectArgs
    {
        VkBuffer buffer;
        VkDeviceSize offset;
        uint32_t drawCount;
        uint32_t stride;
        VkBuffer countBuffer;
        VkDeviceSize countBufferOffset;
    };

    // TODO: Add Args
    struct DispatchArgs
    {

    };

    /// @brief Stores commands for lazy recording
    class CommandRecordingContext
    {
    private:
        std::vector<drawCommand> m_drawCommands;
        std::vector<computeCommand> m_computeCommands;
        std::vector<transferCommand> m_transferCommands;
    public:
        VkDevice m_device;

        CommandRecordingContext(VkDevice device);
        void Release();

        // Graphics & Compute commands
        void BindRenderingInfo(const RenderingInfo& renderingInfo);
        void BindIndexBuffer(const std::shared_ptr<Buffer>& buffer);
        void BindVertexBuffers(const std::vector<std::shared_ptr<Buffer>>& buffers);
        void BindPipeline(const std::shared_ptr<Pipeline>& pipeline);
        void BindDescriptorTable(const std::shared_ptr<DescriptorTable>& table);

        // Graphics commands
        // void ClearColorImage();
        // void ClearDepthStencilImage();
        // void ClearAttachments();

        // void Draw();
        // void DrawMeshTasks();

        void DrawIndexed(const DrawIndexedArgs& args);
        void DrawIndirect(const DrawIndirectArgs& args);
        void Dispatch(const DispatchArgs& args);
    };
}
#endif // LETTUCE_CORE_COMMAND_RECORDING_CONTEXT_HPP