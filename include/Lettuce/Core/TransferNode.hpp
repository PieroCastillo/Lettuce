/*
Created by @PieroCastillo on 2025-07-21
*/
#ifndef LETTUCE_CORE_RENDER_NODE_HPP
#define LETTUCE_CORE_RENDER_NODE_HPP 

namespace Lettuce::Core
{
    struct TransferNodeCreateInfo
    {

    };

    class TransferNode
    {
    private:
        std::function<void(CommandList)> commands;
    public:
        VkDevice m_device;
        TransferNode(VkDevice device, const TransferNodeCreateInfo& createInfo);
        void Release();

        void Record(std::function<void(CommandList)> commands);
        void Replay(VkCommandBuffer commandBuffer);
    };
}
#endif // LETTUCE_CORE_TRANSFER_NODE_HPP