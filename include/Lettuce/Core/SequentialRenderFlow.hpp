/*
Created by @PieroCastillo on 2025-07-27
*/
#ifndef LETTUCE_CORE_SEQUENTIAL_RENDER_FLOW_HPP
#define LETTUCE_CORE_SEQUENTIAL_RENDER_FLOW_HPP

namespace Lettuce::Core
{
    class SequentialRenderFlow
    {
    private:
    
    public:
        VkDevice m_device;

        SequentialRenderFlow(VkDevice device);
        void Release();
    };
}
#endif // LETTUCE_CORE_SEQUENTIAL_RENDER_FLOW_HPP