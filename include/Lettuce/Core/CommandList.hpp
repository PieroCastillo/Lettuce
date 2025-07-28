/*
Created by @PieroCastillo on 2025-07-28
*/
#ifndef LETTUCE_CORE_COMMAND_LIST_HPP
#define LETTUCE_CORE_COMMAND_LIST_HPP 

namespace Lettuce::Core
{
    class CommandList
    {
    private:
    
    public:
        VkDevice m_device;
        VkCommandBuffer m_cmd;

        CommandList(VkDevice device, VkCommandBuffer commandBuffer);
        void Release();
    };
}
#endif // LETTUCE_CORE_COMMAND_LIST_HPP