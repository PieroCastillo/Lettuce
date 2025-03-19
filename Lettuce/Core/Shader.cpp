//
// Created by piero on 6/03/2025.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "Lettuce/Core/Descriptors.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/Shader.hpp"

using namespace Lettuce::Core;

void Shader::AddConstant(uint32_t constantId, uint32_t size, void *data)
{
    VkSpecializationMapEntry entry = {
        .constantID = constantId,
        .offset = currentOffset,
        .size = size,
    };
    entries.push_back(entry);
    datas.push_back(data);

    currentOffset += size;
}

void Shader::Assemble(const VkShaderStageFlagBits &stage,
                      const std::string &entryPoint,
                      const std::vector<uint32_t> &code, 
                      const VkShaderStageFlags &nextStage,
                      const VkShaderCreateFlagsEXT &flags)
{
    VkShaderCreateInfoEXT shaderCI = {
        .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
        .flags = flags,
        .stage = stage,
        .nextStage = nextStage,
        .codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT, // this is the most common format
        .codeSize = sizeof(uint32_t) * code.size(),
        .pCode = (void *)code.data(),
        .pName = (char *)entryPoint.data(),
        .setLayoutCount = (uint32_t)_layout->_descriptors->_layouts.size(),
        .pSetLayouts = _layout->_descriptors->_layouts.data(),
        .pushConstantRangeCount = (uint32_t)_layout->pushConstants.size(),
        .pPushConstantRanges = _layout->pushConstants.data(),
    };

    // here we add the specialization constants
    void *data;
    if (entries.size() > 0)
    {
        VkSpecializationInfo info;
        info.mapEntryCount = (uint32_t)entries.size();
        info.pMapEntries = entries.data();

        uint32_t fullsize = 0;
        for (auto &entry : entries)
        {
            fullsize += entry.size;
        }

        int i = 0;
        data = malloc(fullsize);
        char *iter = (char *)data;
        for (auto &partialData : datas)
        {
            memcpy((void *)iter, partialData, entries[i].size);
            iter += entries[i].size;
            i++;
        }
        info.dataSize = fullsize;
        info.pData = data;
        shaderCI.pSpecializationInfo = &info;
    }
    checkResult(vkCreateShadersEXT(_device->_device, 1, &shaderCI, nullptr, &_shader));
    if (entries.size() > 0)
    {
        free(data);
    }
}

void Shader::Release()
{
    vkDestroyShaderEXT(_device->_device, _shader, nullptr);
}