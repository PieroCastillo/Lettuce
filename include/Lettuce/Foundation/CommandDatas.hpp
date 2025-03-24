//
// Created by piero on 22/03/2025.
//
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "WorkState.hpp"
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/ImageResource.hpp"

namespace Lettuce::Foundation
{
    // NEEDS REVIEW

    enum class ExecutionMode
    {
        Draw,
        DrawIndexed,
        DrawIndirect,
        DrawIndexedIndirect,
        DrawCount,
        DrawIndexedCount,
        DrawIndirectCount,
        DrawIndexedIndirectCount,
        DrawMeshTasks,
        DrawMeshTasksIndirect,
        DrawMeshTasksIndirectCount,

        Dispatch,
        DispatchIndirect,
    };

    struct DrawCommandData
    {
    };
    struct DrawIndexedCommandData
    {
    };
    struct DrawIndirectCommandData
    {
    };
    struct DrawIndexedIndirectCommandData
    {
    };
    struct DrawCountCommandData
    {
    };
    struct DrawIndexedCountCommandData
    {
    };
    struct DrawIndirectCountCommandData
    {
    };
    struct DrawIndexedIndirectCountCommandData
    {
    };
    struct DrawMeshTasksCommandData
    {
    };
    struct DrawMeshTasksIndirectCommandData
    {
    };
    struct DrawMeshTasksIndirectCountCommandData
    {
    };

    struct DispatchCommandData
    {
    };
    struct DispatchIndirectCommandData
    {
    };

    using CommandData = std::variant<DrawCommandData,
                                     DrawIndexedCommandData,
                                     DrawIndirectCommandData,
                                     DrawIndexedIndirectCommandData,
                                     DrawCountCommandData,
                                     DrawIndexedCountCommandData,
                                     DrawIndirectCountCommandData,
                                     DrawIndexedIndirectCountCommandData,
                                     DrawMeshTasksCommandData,
                                     DrawMeshTasksIndirectCommandData,
                                     DrawMeshTasksIndirectCountCommandData,
                                     DispatchCommandData,
                                     DispatchIndirectCommandData>;
}