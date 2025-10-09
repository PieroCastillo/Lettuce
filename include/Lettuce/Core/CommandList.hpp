/*
Created by @PieroCastillo on 2025-07-21
*/
#ifndef LETTUCE_CORE_COMMAND_LIST_HPP
#define LETTUCE_CORE_COMMAND_LIST_HPP

// standard headers
#include <functional>

// project headers
#include "common.hpp"
#include "CommandRecordingContext.hpp"

namespace Lettuce::Core
{
    enum class CommandListType
    {
        Graphics,
        Compute,
        Transfer,
    };

    struct CommandListCreateInfo
    {
        CommandListType type;
    };

    /*
    CommandList stores (via a CommandRecordingContext) a list of commands to be recorded later.
    */
    class CommandList
    {
    private:
    VkCommandBuffer commandBuffer;
        CommandRecordingContext commands;

    public:
        CommandListType type;
        
        void Record(std::function<void(CommandRecordingContext&)> commands);
        void Replay(VkCommandBuffer commandBuffer);
    };
}
#endif // LETTUCE_CORE_COMMAND_LIST_HPP