//
// Created by piero on 2/11/2024.
//
#include "Lettuce/Core/common.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/ShaderModule.hpp"
#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Descriptors.hpp"
#include "Lettuce/Core/Compilers/ICompiler.hpp"
#include "Lettuce/X2D/Materials/MaterialBase.hpp"
#include "Lettuce/X2D/Geometries/Vertex.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::X2D::Materials;

template<typename T, typename TPushData>
void MaterialBase<T,TPushData>::Load(const std::shared_ptr<Device> &device, RenderPass &renderpass, Compilers::ICompiler &compiler)
{
    MaterialBase<T,TPushData> &material = static_cast<T*>(this);

    layout.AddPushConstant<TPushData>(0, PipelineStage::Fragment);
    layout = std::make_shared<>(device, *descriptorsPtr);
    ShaderModule frag, vert;
    frag = std::make_shared<>(device, compiler, material.GetFragmentShaderText(), "main", "fragment.glsl", PipelineStage::Fragment, true);
    vert = std::make_shared<>(device, compiler, material.GetVertexShaderText(), "main", "vertex.glsl", PipelineStage::Vertex, true);

    pipeline.AddVertexBindingDescription<Lettuce::X2D::Geometries::Vertex>(0);
    pipeline.AddVertexAttribute(0, 0, 0, VK_FORMAT_R32G32_SFLOAT);
    pipeline.AddVertexAttribute(0, 1, sizeof(glm::vec2), VK_FORMAT_R32G32_SFLOAT);
    pipeline.AddShaderStage(frag);
    pipeline.AddShaderStage(vert);
    pipeline = std::make_shared<>(device, this->layout, renderpass, 0,
                   {.rasterization = {
                        .frontFace = VK_FRONT_FACE_CLOCKWISE,
                    },
                    .colorBlend = {
                        .attachments = {
                            {
                                .colorWriteMask = VK_COMPONENT_SWIZZLE_R | VK_COMPONENT_SWIZZLE_G | VK_COMPONENT_SWIZZLE_B | VK_COMPONENT_SWIZZLE_A,
                            },
                        },
                    }});

    frag.Destroy();
    vert.Destroy();
}

template<typename T, typename TPushData>
void MaterialBase<T,TPushData>::Release()
{
    pipeline.Destroy();
}