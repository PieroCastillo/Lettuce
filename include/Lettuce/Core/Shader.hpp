//
// Created by piero on 14/02/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Device.hpp"
#include "Utils.hpp"

namespace Lettuce::Core
{
    class Shader
    {
    public:
        std::shared_ptr<Device> _device;
        PipelineStage _stage;
        VkShaderModule _shaderModule;
        std::string _name;

        template <class T = Compilers::ICompiler>
        void Create(const std::shared_ptr<Device> &device, T &compiler, std::string text, std::string mainMethod, std::string inputFile, PipelineStage stage, bool optimize = false)
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
            checkResult(vkCreateShaderModule(_device->_device, &shaderModuleCI, nullptr, &_shaderModule));
        }

        void Destroy();
    };
}