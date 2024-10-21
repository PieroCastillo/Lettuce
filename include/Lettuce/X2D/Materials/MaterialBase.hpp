//
// Created by piero on 30/09/2024.
//
#include "Lettuce/Core/Descriptors.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/GraphicsPipeline.hpp"

using namespace Lettuce::Core;

namespace Lettuce::X2D::Materials
{
    class MaterialBase
    {
        public:
        GraphicsPipeline pipeline;
    };
}