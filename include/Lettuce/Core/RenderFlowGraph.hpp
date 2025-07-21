/*
Created by @PieroCastillo on 2025-07-21
*/
#ifndef LETTUCE_CORE_RENDER_FLOW_GRAPH_HPP
#define LETTUCE_CORE_RENDER_FLOW_GRAPH_HPP 

namespace Lettuce::Core
{
    struct RenderFlowGraphCreateInfo
    {

    };

    class RenderFlowGraph
    {
    private:

    public:
        VkDevice m_device;
        RenderFlowGraph(VkDevice device, const RenderFlowGraphCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_RENDER_FLOW_GRAPH_HPP