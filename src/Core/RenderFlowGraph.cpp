// standard headers
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/RenderFlowGraph.hpp"

using namespace Lettuce::Core;

void RenderNode::LinkTo(RenderNode& node)
{
    nextNodes.push_back(&node);
}

void RenderFlowGraph::Create(const IDevice& device, const RenderFlowGraphCreateInfo& createInfo)
{
    m_device = device.m_device;
}

void RenderFlowGraph::Release()
{
    m_nodes.clear();
    m_commands.clear();
}

RenderNode& RenderFlowGraph::CreateNode(NodeKind kind, std::function<void(const CommandRecordingContext&)> record)
{
    auto node = std::make_unique<RenderNode>();
    node->kind = kind;
    node->func = std::move(record);

    RenderNode& ref = *node;
    m_nodes.push_back(std::move(node));
    return ref;
}

void RenderFlowGraph::Compile()
{

}

CommandsList RenderFlowGraph::GetCommands()
{
    if(!m_readyForRecording) [[unlikely]]
    {
        throw LettuceException(LettuceResult::NotReady);
    }

    return m_commands;
}