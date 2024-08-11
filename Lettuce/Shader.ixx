//
// Created by piero on 14/02/2024.
//
module;
#include <iostream>
#include <vector>
#include <string>
#include "volk.h"

export module Lettuce:Shader;

import :Device;
import :PipelineConnector;
import :Utils;
import :ICompiler;

export namespace Lettuce::Core
{
    class Shader
    {
    public:
        Device _device;
        PipelineStage _stage;
        VkShaderModule _shaderModule;
        std::string _name;

        template <class T = Compilers::Compiler>
        void Create(Device &device, T &compiler, std::string text, std::string mainMethod, std::string inputFile, PipelineStage stage, bool optimize = false)
        {
            _device = device;
            _stage = stage;
            _name = mainMethod;

            auto code = compiler.Compile(text, inputFile, stage, optimize);

            VkShaderModuleCreateInfo shaderModuleCI = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = code.size() * sizeof(uint32_t),
                .pCode = code.data(),
            };
            checkResult(vkCreateShaderModule(_device._device, &shaderModuleCI, nullptr, &_shaderModule), "shader module created sucessfully");
        }

        void Destroy()
        {
            vkDestroyShaderModule(_device._device, _shaderModule, nullptr);
        }
    };
}