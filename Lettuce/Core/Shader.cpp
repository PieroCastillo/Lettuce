//
// Created by piero on 14/02/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include <string>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/Shader.hpp"
#include "Lettuce/Core/Compilers/ICompiler.hpp"

using namespace Lettuce::Core;

Shader::~Shader()
{
    vkDestroyShaderModule(_device->_device, _shaderModule, nullptr);
}