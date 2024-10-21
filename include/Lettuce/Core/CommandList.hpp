//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <cstdint>
#include "GraphicsPipeline.hpp"
#include "ComputePipeline.hpp"
#include "PipelineLayout.hpp"
#include "Buffer.hpp"
#include "Swapchain.hpp"
#include "Device.hpp"
#include "Utils.hpp"

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

        void Begin();

        void End();

        void BindGraphicsPipeline(GraphicsPipeline &pipeline);

        void BindDescriptorSetToGraphics(PipelineLayout &connector, Descriptors &descriptor);

        void BindDescriptorSetToCompute(PipelineLayout &connector, Descriptors &descriptor);

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
        void PushConstant(PipelineLayout &connector, PipelineStage stage, T &constant, uint32_t offset = 0);

        void Reset();
    };
}