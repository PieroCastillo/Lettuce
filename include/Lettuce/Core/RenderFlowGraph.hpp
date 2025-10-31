/*
Created by @PieroCastillo on 2025-07-21
*/
#ifndef LETTUCE_CORE_RENDER_FLOW_GRAPH_HPP
#define LETTUCE_CORE_RENDER_FLOW_GRAPH_HPP

// standard headers
#include <atomic>
#include <concepts>
#include <functional>
#include <memory>
#include <variant>
#include <vector>

// project headers
#include "common.hpp"
#include "CommandRecordingContext.hpp"

namespace Lettuce::Core
{
    enum class NodeKind
    {
        Graphics,
        Compute
    };

    template<typename T>
    concept ICommandRecordingContext = requires(T ctx, VkCommandBuffer cmd)
    {
        { ctx.record(cmd) } -> std::same_as<void>;
    };

    struct RenderFlowGraphCreateInfo
    {
    };

    template<ICommandRecordingContext... Contexts>
    class RenderFlowGraph
    {
    private:
    public:
        template<ICommandRecordingContext T>
        struct RenderNode
        {
            NodeKind kind;
            T context;
            std::function<void(const CommandRecordingContext&)> func;
            std::vector<std::variant<RenderNode<Contexts>...>*> nextNodes;

            template<ICommandRecordingContext U>
            void LinkTo(const RenderNode<U>& node);
        };

        VkDevice m_device;
        std::vector<std::unique_ptr<std::variant<RenderNode<Contexts>...>>> m_nodes;
        CommandsList m_commands;
        std::atomic<bool> m_readyForRecording = false;

        void Create(const IDevice& device, const RenderFlowGraphCreateInfo& createInfo);
        void Release();

        template<ICommandRecordingContext T>
        RenderNode<T>& CreateNode(NodeKind kind, std::function<void(const T&)> record);

        // sort nodes
        void Compile();
        // record to command buffers
        CommandsList GetCommands();
    };
}

#include "RenderFlowGraph.inl"
#endif // LETTUCE_CORE_RENDER_FLOW_GRAPH_HPP