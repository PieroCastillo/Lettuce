//
// Created by piero on 14/02/2024.
//
#include <iostream>
#include <volk.h>
#include <vector>
#include <string>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/Shader.hpp"

using namespace Lettuce::Core;

template <class T = Compilers::Compiler>
void Shader::Create(Device &device, T &compiler, std::string text, std::string mainMethod, std::string inputFile, PipelineStage stage, bool optimize = false)
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

void Shader::Destroy()
{
    vkDestroyShaderModule(_device._device, _shaderModule, nullptr);
}