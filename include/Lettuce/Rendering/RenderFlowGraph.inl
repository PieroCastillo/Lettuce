/*
Created by @PieroCastillo on 2025-10-31
*/
#ifndef LETTUCE_CORE_RENDER_FLOW_GRAPH_INL
#define LETTUCE_CORE_RENDER_FLOW_GRAPH_INL

#include "RenderFlowGraph.hpp"

using namespace Lettuce::Core;

template<ICommandRecordingContext... Contexts>
void RenderFlowGraph<Contexts...>::Create(const IDevice& device, const RenderFlowGraphCreateInfo& createInfo)
{
    m_device = device.m_device;
    m_readyForRecording = false;
    // inicialización de nodos, etc.
}

template<ICommandRecordingContext... Contexts>
void RenderFlowGraph<Contexts...>::Release()
{
    m_nodes.clear();
    m_commands.clear();
    m_readyForRecording = false;
}

template<ICommandRecordingContext... Contexts>
template<ICommandRecordingContext T>
typename RenderFlowGraph<Contexts...>::template RenderNode<T>&
RenderFlowGraph<Contexts...>::CreateNode(NodeKind kind, std::function<void(const T&)> record)
{
    auto node = std::make_unique<std::variant<RenderNode<Contexts>...>>(RenderNode<T>{kind, {}, record});
    auto& ref = std::get<RenderNode<T>>(*node);
    m_nodes.push_back(std::move(node));
    return ref;
}

template<ICommandRecordingContext... Contexts>
template<ICommandRecordingContext T>
template<ICommandRecordingContext U>
void RenderFlowGraph<Contexts...>::RenderNode<T>::LinkTo(const RenderNode<U>& node)
{
    nextNodes.push_back(reinterpret_cast<std::variant<RenderNode<Contexts>...>*>(&node));
}

template<ICommandRecordingContext... Contexts>
void RenderFlowGraph<Contexts...>::Compile()
{
    // sort nodes
}

template<ICommandRecordingContext... Contexts>
CommandsList RenderFlowGraph<Contexts...>::GetCommands()
{
    if(!m_readyForRecording) [[unlikely]]
    {
        throw LettuceException(LettuceResult::NotReady);
    }

    return m_commands;
}
#endif // LETTUCE_CORE_RENDER_FLOW_GRAPH_INL