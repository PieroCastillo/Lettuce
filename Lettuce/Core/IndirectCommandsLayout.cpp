//
// Created by piero on 6/03/2025.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/IndirectCommandsLayout.hpp"

using namespace Lettuce::Core;

void IndirectCommandsLayout::Assemble() 
{
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkIndirectCommandsTokenTypeEXT.html token-commands relation

    VkIndirectCommandsLayoutCreateInfoEXT layoutCI = {
        .sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_EXT,
        // .pNext = ,
        // .flags = ,
        // .shaderStages = ,
        // .indirectStride = ,
        // .pipelineLayout = ,
        // .tokenCount = ,
        // .pTokens = ,
    };
}

void IndirectCommandsLayout::Release()
{
    vkDestroyIndirectCommandsLayoutEXT(_device->_device, _commandsLayout, nullptr);
}