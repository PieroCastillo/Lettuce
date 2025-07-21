/*
Created by @PieroCastillo on 2025-07-21
*/
#ifndef LETTUCE_CORE_COMPUTE_NODE_HPP
#define LETTUCE_CORE_COMPUTE_NODE_HPP 

namespace Lettuce::Core
{
    struct ComputeNodeCreateInfo
    {

    };

    class ComputeNode
    {
    private:

    public:
        VkDevice m_device;
        ComputeNode(VkDevice device, const ComputeNodeCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_COMPUTE_NODE_HPP