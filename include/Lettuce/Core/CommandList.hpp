//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <volk.h>

namespace Lettuce::Core
{
    enum class IndexType
    {
        UInt16 = 0,
        UInt32 = 1,
        UInt8 = 1000265000, // Provided by VK_KHR_index_type_uint8
    };

    enum class Topology
    {
        PointList = 0,
        LineList = 1,
        LineStrip = 2,
        TriangleList = 3,
        TriangleStrip = 4,
        TriangleFan = 5,
        LineListWithAdjacency = 6,
        LineStripWithAdjacency = 7,
        TriangleListWithAdjacency = 8,
        TriangleStripWithAdjacency = 9,
        PatchList = 10,
    };

    /// @brief This is a interface to use VkCommandBuffer
    class CommandList
    {
    public:
        VkCommandBuffer _commandBuffer;
        Device _device;

        void Create(Device &device);

        void Begin();

        void End();

        void BeginRendering(Swapchain swapchain, float r = 1, float g = 1, float b = 1, float a = 1);

        void EndRendering(Swapchain swapchain);

        /// @brief send CommandLists to the Queue, use VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT when the CommandLists are going to be presented
        /// @param cmds
        /// @param waitStages
        /// @param semaphores
        /// @param waitValues
        /// @param signalValues
        static void Send(Device device, std::vector<CommandList> cmds,
                         AccessStage waitStage,
                         std::vector<TSemaphore> semaphores,
                         std::vector<uint64_t> waitValues,
                         std::vector<uint64_t> signalValues);

        void BindGraphicsPipeline(GraphicsPipeline &pipeline);

        void BindDescriptorSetToGraphics(PipelineConnector &connector, Descriptor &descriptor);

        void BindDescriptorSetToCompute(PipelineConnector &connector, Descriptor &descriptor);

        void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, std::vector<Buffer> &buffers, std::vector<uint32_t> &offsets);

        void BindVertexBuffer(Buffer &vertexBuffer);

        void BindIndexBuffer(Buffer &indexBuffer, IndexType indexType);

        void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);

        void DrawIndirect(Buffer indirectBuffer, uint32_t drawCount, uint32_t stride, uint32_t offset = 0);

        void DrawMesh(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

        void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

        void SetScissor(Swapchain swapchain, int32_t xOffset = 0, int32_t yOffset = 0);

        void SetViewport(float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f, float x = 0.0f, float y = 0.0f);

        void SetLineWidth(float lineWidth);

        void SetTopology(Topology topology);

        template <typename T>
        void PushConstant(PipelineConnector &connector, PipelineStage stage, T &constant, uint32_t offset = 0);

        void Reset();
    };
}