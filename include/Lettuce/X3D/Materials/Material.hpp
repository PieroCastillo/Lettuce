//
// Created by piero on 7/04/2025.
//
#include <string>
#include <memory>

#include "Lettuce/Core/ShaderModule.hpp"

using Lettuce::Core;

namespace Lettuce::Geometry::Materials
{
    class Material
    {
        std::string materialName;
        uint32_t materialID;
        std::shared_ptr<ShaderModule> fragmentModule;
    };
}