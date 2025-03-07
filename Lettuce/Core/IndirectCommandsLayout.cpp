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
    // VkStructureType                            sType;
    //     const void*                                pNext;
    //     VkIndirectCommandsLayoutUsageFlagsEXT      flags;
    //     VkShaderStageFlags                         shaderStages;
    //     uint32_t                                   indirectStride;
    //     VkPipelineLayout                           pipelineLayout;
    //     uint32_t                                   tokenCount;
    //     const VkIndirectCommandsLayoutTokenEXT*    pTokens;
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
