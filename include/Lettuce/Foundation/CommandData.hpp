//
// Created by piero on 22/03/2025.
//
#pragma once
#include <string>
#include <vector>
#include <type_traits>
#include "BufferHandle.hpp"
#include "ImageHandle.hpp"

namespace Lettuce::Foundation
{
    enum class ExecutionMode
    {
        Draw,
        DrawIndexed,
        DrawIndexedIndirect,
        DrawIndexedIndirectCount,
        DrawIndirect,
        DrawIndirectCount,
        DrawMeshTasks,
        DrawMeshTasksIndirect,
        DrawMeshTasksIndirectCount,

        Dispatch,
        DispatchIndirect,
    };

    struct BaseCommandData
    {
        virtual void record(VkCommandBuffer cmd) = 0;
    };

    struct DrawCommandData : BaseCommandData
    {
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;

        void record(VkCommandBuffer cmd) override
        {
            vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
        }
    };
    struct DrawIndexedCommandData : BaseCommandData
    {
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        int32_t vertexOffset;
        uint32_t firstInstance;

        void record(VkCommandBuffer cmd) override
        {
            vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }
    };
    struct DrawIndexedIndirectCommandData : BaseCommandData
    {
        BufferHandle buffer;
        uint64_t offset;
        uint32_t drawCount;
        uint32_t stride;

        void record(VkCommandBuffer cmd) override
        {
            vkCmdDrawIndexedIndirect(cmd, buffer.buffer->_buffer, offset, drawCount, stride);
        }
    };
    struct DrawIndexedIndirectCountCommandData : BaseCommandData
    {
        BufferHandle buffer;
        uint64_t offset;
        BufferHandle countBuffer;
        uint64_t countBufferOffset;
        uint32_t maxDrawCount;
        uint32_t stride;

        void record(VkCommandBuffer cmd) override
        {
            vkCmdDrawIndexedIndirectCount(cmd, buffer.buffer->_buffer, offset, countBuffer.buffer->_buffer, countBufferOffset, maxDrawCount, stride);
        }
    };
    struct DrawIndirectCommandData : BaseCommandData
    {
        BufferHandle buffer;
        uint64_t offset;
        uint32_t drawCount;
        uint32_t stride;

        void record(VkCommandBuffer cmd) override
        {
            vkCmdDrawIndirect(cmd, buffer.buffer->_buffer, offset, drawCount, stride);
        }
    };
    struct DrawIndirectCountCommandData : BaseCommandData
    {
        BufferHandle buffer;
        uint64_t offset;
        BufferHandle countBuffer;
        uint64_t countBufferOffset;
        uint32_t maxDrawCount;
        uint32_t stride;

        void record(VkCommandBuffer cmd) override
        {
            vkCmdDrawIndirectCount(cmd, buffer.buffer->_buffer, offset, countBuffer.buffer->_buffer, countBufferOffset, maxDrawCount, stride);
        }
    };

    struct DrawMeshTasksCommandData : BaseCommandData
    {
        uint32_t groupCountX;
        uint32_t groupCountY;
        uint32_t groupCountZ;

        void record(VkCommandBuffer cmd) override
        {
            vkCmdDrawMeshTasksEXT(cmd, groupCountX, groupCountY, groupCountZ);
        }
    };
    struct DrawMeshTasksIndirectCommandData : BaseCommandData
    {
        BufferHandle buffer;
        uint64_t offset;
        uint32_t drawCount;
        uint32_t stride;

        void record(VkCommandBuffer cmd) override
        {
            vkCmdDrawMeshTasksIndirectEXT(cmd, buffer.buffer->_buffer, offset, drawCount, stride);
        }
    };
    struct DrawMeshTasksIndirectCountCommandData : BaseCommandData
    {
        BufferHandle buffer;
        uint64_t offset;
        BufferHandle countBuffer;
        uint64_t countBufferOffset;
        uint32_t maxDrawCount;
        uint32_t stride;

        void record(VkCommandBuffer cmd) override
        {
            vkCmdDrawMeshTasksIndirectCountEXT(cmd, buffer.buffer->_buffer, offset, countBuffer.buffer->_buffer, countBufferOffset, maxDrawCount, stride);
        }
    };

    struct DispatchCommandData : BaseCommandData
    {
        uint32_t groupCountX;
        uint32_t groupCountY;
        uint32_t groupCountZ;
        void record(VkCommandBuffer cmd) override
        {
            vkCmdDispatch(cmd, groupCountX, groupCountY, groupCountZ);
        }
    };
    struct DispatchIndirectCommandData : BaseCommandData
    {
        uint32_t baseGroupX;
        uint32_t baseGroupY;
        uint32_t baseGroupZ;
        uint32_t groupCountX;
        uint32_t groupCountY;
        uint32_t groupCountZ;

        void record(VkCommandBuffer cmd) override
        {
            vkCmdDispatchBase(cmd, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
        }
    };

    template <typename T>
    concept CommandData =
        (std::is_same_v<std::remove_cvref_t<T>, DrawCommandData> ||
         std::is_same_v<std::remove_cvref_t<T>, DrawIndexedCommandData> ||
         std::is_same_v<std::remove_cvref_t<T>, DrawIndirectCommandData> ||
         std::is_same_v<std::remove_cvref_t<T>, DrawIndexedIndirectCommandData> ||
         std::is_same_v<std::remove_cvref_t<T>, DrawIndirectCountCommandData> ||
         std::is_same_v<std::remove_cvref_t<T>, DrawIndexedIndirectCountCommandData> ||
         std::is_same_v<std::remove_cvref_t<T>, DrawMeshTasksCommandData> ||
         std::is_same_v<std::remove_cvref_t<T>, DrawMeshTasksIndirectCommandData> ||
         std::is_same_v<std::remove_cvref_t<T>, DrawMeshTasksIndirectCountCommandData> ||
         std::is_same_v<std::remove_cvref_t<T>, DispatchCommandData> ||
         std::is_same_v<std::remove_cvref_t<T>, DispatchIndirectCommandData>) &&
        requires(T &t, VkCommandBuffer cmd) {
            { t.record(cmd) } -> std::same_as<void>;
        };
}