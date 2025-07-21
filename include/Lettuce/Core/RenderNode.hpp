/*
Created by @PieroCastillo on 2025-07-21
*/
#ifndef LETTUCE_CORE_RENDER_NODE_HPP
#define LETTUCE_CORE_RENDER_NODE_HPP 

namespace Lettuce::Core
{
    struct RenderNodeCreateInfo
    {

    };

    class RenderNode
    {
    private:

    public:
        VkDevice m_device;
        RenderNode(VkDevice device, const RenderNodeCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_RENDER_NODE_HPP