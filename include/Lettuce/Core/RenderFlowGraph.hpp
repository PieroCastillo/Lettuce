/*
Created by @PieroCastillo on 2025-07-21
*/
#ifndef LETTUCE_CORE_RENDER_FLOW_GRAPH_HPP
#define LETTUCE_CORE_RENDER_FLOW_GRAPH_HPP

// standard headers
#include <functional>
#include <memory>
#include <atomic>
#include <vector>
#include <variant>

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

    struct RenderNode
    {
        NodeKind kind;
        std::function<void(const CommandRecordingContext&)> func;
        std::vector<RenderNode*> nextNodes;
        void LinkTo(RenderNode& node);
    };

    struct RenderFlowGraphCreateInfo
    {
    };

    class RenderFlowGraph
    {
    private:
    public:
        VkDevice m_device;
        std::vector<std::unique_ptr<RenderNode>> m_nodes;
        CommandsList m_commands;
        std::atomic<bool> m_readyForRecording = false;

        void Create(const IDevice& device, const RenderFlowGraphCreateInfo& createInfo);
        void Release();

        RenderNode& CreateNode(NodeKind kind, std::function<void(const CommandRecordingContext&)> record);
        // sort nodes
        void Compile();
        // record to command buffers
        CommandsList GetCommands();
    };
}
#endif // LETTUCE_CORE_RENDER_FLOW_GRAPH_HPP