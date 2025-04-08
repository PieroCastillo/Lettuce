//
// Created by piero on 14/10/2024.
//
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Lettuce/Core/Common.hpp"
#include "Lettuce/X2D/Materials/MaterialBase.hpp"
#include "Lettuce/X2D/Materials/ColorMaterial.hpp"

using namespace Lettuce::X2D::Materials;

void ColorMaterial::UpdateDescriptors() {}
std::string ColorMaterial::GetFragmentShaderText()
{
    return "";
}
std::string ColorMaterial::GetVertexShaderText()
{
    return "";
}