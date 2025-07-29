/*
Created by @PieroCastillo on 2025-07-21
*/
#ifndef LETTUCE_CORE_COMPUTE_NODE_HPP
#define LETTUCE_CORE_COMPUTE_NODE_HPP

// standard headers
#include <function>

// project headers
#include "CommandList.hpp"

namespace Lettuce::Core
{
    struct ComputeNodeCreateInfo
    {
    };

    class ComputeNode
    {
    private:
        std::function<void(CommandList)> commands;

    public:
        VkDevice m_device;
        ComputeNode(VkDevice device, const ComputeNodeCreateInfo &createInfo);
        void Release();

        void Record(std::function<void(CommandList)> commands);
        void Replay(VkCommandBuffer commandBuffer);
    };
}
#endif // LETTUCE_CORE_COMPUTE_NODE_HPP