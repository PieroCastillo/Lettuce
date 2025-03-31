//
// Created by piero on 22/03/2025.
//
#pragma once
#include <vector>
#include <memory>
#include <variant>
#include <string>
#include <utility>
#include <vector>
#include "CommandState.hpp"
#include "CommandData.hpp"
#include "BufferHandle.hpp"
#include "ImageHandle.hpp"

namespace Lettuce::Foundation
{
    class WorkFlowGraph;

    template <CommandData T>
    struct WorkNodeDataT
    {
        std::string _name;
        std::vector<T> _commands;
        std::vector<CommandState> _states;
    };

    using WorkNodeData = std::variant<WorkNodeDataT<DrawCommandData>,
                                      WorkNodeDataT<DrawIndexedCommandData>,
                                      WorkNodeDataT<DrawIndirectCommandData>,
                                      WorkNodeDataT<DrawIndexedIndirectCommandData>,
                                      WorkNodeDataT<DrawIndirectCountCommandData>,
                                      WorkNodeDataT<DrawIndexedIndirectCountCommandData>,
                                      WorkNodeDataT<DrawMeshTasksCommandData>,
                                      WorkNodeDataT<DrawMeshTasksIndirectCommandData>,
                                      WorkNodeDataT<DrawMeshTasksIndirectCountCommandData>,
                                      WorkNodeDataT<DispatchCommandData>,
                                      WorkNodeDataT<DispatchIndirectCommandData>>;

    class WorkNode
    {
    private:
        friend class WorkFlowGraph;

        struct Edge
        {
            std::shared_ptr<WorkNode> child;
            std::variant<BufferHandle, ImageHandle> resourceHandle;
        };
        std::vector<Edge> children;
        uint32_t level = 0;
        WorkNodeData data;

        template <CommandData T>
        explicit WorkNode(const std::string &name,
                          const std::vector<T> &commands,
                          const std::vector<CommandState> &states)
        {
            data = {name, commands, states};
        }

    public:
        virtual ~WorkNode() = default;

        void LinkTo(const std::shared_ptr<WorkNode> &dst, std::variant<BufferHandle, ImageHandle> resourceHandle)
        {
            children.push_back(Edge{dst, resourceHandle});
        }
    };
}