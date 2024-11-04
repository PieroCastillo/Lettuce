//
// Created by piero on 30/09/2024.
//
#pragma once
#include <memory>
#include "IMaterial.hpp"
#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Descriptors.hpp"
#include "Lettuce/Core/RenderPass.hpp"
#include "Lettuce/Core/Compilers/ICompiler.hpp"

using namespace Lettuce::Core;

namespace Lettuce::X2D::Materials
{
    /// @brief MaterialBase represents a Material,
    /// this class follows the CRTP pattern
    /// @tparam T Derivated of MaterialBase
    /// @tparam TPushData
    template <typename T, typename TPushData>
    class MaterialBase : public IMaterial
    {
    public:
        MaterialBase(T *material)
        {
            // Implementación
        }
        virtual std::string GetFragmentShaderText();
        virtual std::string GetVertexShaderText();
        // virtual void PushData(TPushData data);
        void Load(Device &device, RenderPass &renderpass, Compilers::ICompiler &compiler);
        void Release();
    };
}