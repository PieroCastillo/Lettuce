//
// Created by piero on 30/09/2024.
//
#pragma once
#include <memory>
#include "Lettuce/Core/Descriptors.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Descriptors.hpp"

using namespace Lettuce::Core;

namespace Lettuce::X2D::Materials
{
    class MaterialBase
    {
        public:
        std::shared_ptr<Descriptors> descriptorsPtr;
        PipelineLayout layout;
        GraphicsPipeline pipeline;
    };
}