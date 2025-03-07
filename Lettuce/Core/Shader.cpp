//
// Created by piero on 6/03/2025.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include "Lettuce/Core/Descriptors.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/Shader.hpp"

using namespace Lettuce::Core;

void Shader::Assemble(const VkShaderStageFlagBits& stage, const std::shared_ptr<Descriptors>& descriptors)
{
    //    VkStructureType sType;
    // const void *pNext;
    // VkShaderCreateFlagsEXT flags;
    // ;
    // VkShaderStageFlags nextStage;
    // VkShaderCodeTypeEXT codeType;
    // size_t codeSize;
    // const void *pCode;
    // const char *pName;
    // uint32_t setLayoutCount;
    // const VkDescriptorSetLayout *pSetLayouts;
    // uint32_t pushConstantRangeCount;
    // const VkPushConstantRange *pPushConstantRanges;
    // const VkSpecializationInfo *pSpecializationInfo;
    VkShaderCreateInfoEXT shaderCI = {
        .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
        .stage = stage,

        
        .setLayoutCount = (uint32_t)descriptors->_layouts.size(),
        .pSetLayouts = descriptors->_layouts.data(),
    };
}