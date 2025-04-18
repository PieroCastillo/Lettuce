//
// Created by piero on 14/02/2024.
//
#include "Lettuce/Core/Common.hpp"
#include <iostream>
#include <vector>
#include <string>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/ShaderModule.hpp"
#include "Lettuce/Core/Compilers/ICompiler.hpp"

using namespace Lettuce::Core;

void ShaderModule::Release()
{
    vkDestroyShaderModule(_device->GetHandle(), GetHandle(), nullptr);
}