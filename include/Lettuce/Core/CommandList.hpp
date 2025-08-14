/*
Created by @PieroCastillo on 2025-07-21
*/
#ifndef LETTUCE_CORE_COMMAND_LIST_HPP
#define LETTUCE_CORE_COMMAND_LIST_HPP

// standard headers
#include <function>

// project headers
#include "Common.hpp"
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

    class CommandList
    {
    private:
        std::function<void(CommandRecordingContext&)> commands;

    public:
        CommandListType type;
        
        void Record(std::function<void(CommandRecordingContext&)> commands);
        void Replay(VkCommandBuffer commandBuffer);
    };
}
#endif // LETTUCE_CORE_COMMAND_LIST_HPP