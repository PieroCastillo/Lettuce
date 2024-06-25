//
// Created by piero on 14/02/2024.
//
module;
#include <iostream>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <vector>
#include <string>

export module Lettuce:Shader;

import :Device;
import :PipelineConnector;
import :Utils;

export namespace Lettuce::Core
{
    class Shader
    {
    public:
        Device _device;
        LettucePipelineStage _stage;
        VkShaderModule _shaderModule;

        void Create(Device &device)
        {
            _device = device;
        }

        void Compile(std::string text, std::string mainMethod, LettucePipelineStage stage)
        {
            _stage = stage;

            VkShaderModuleCreateInfo shaderModuleCI = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                // size_t                       codeSize;
                // uint32_t*              pCode;
            };
            checkResult(vkCreateShaderModule(_device._device, &shaderModuleCI, nullptr, &_shaderModule), std::to_string(_stage) + " shader module created sucessfully");
        }

        void Destroy()
        {
            vkDestroyShaderModule(_device._device, _shaderModule, nullptr);
        }
    };
}