//
// Created by piero on 30/09/2024.
//
#pragma once
#include "MaterialBase.hpp"
#include "glm/glm.hpp"

namespace Lettuce::X2D::Materials
{
    struct ColorPushData
    {
        glm::vec4 color;
    };

    class ColorMaterial : public MaterialBase<ColorMaterial, ColorPushData>
    {
    public:
        ColorMaterial() : MaterialBase<ColorMaterial, ColorPushData>(this)
        {
        }
        void UpdateDescriptors();
        std::string GetFragmentShaderText();
        std::string GetVertexShaderText();
    };
}