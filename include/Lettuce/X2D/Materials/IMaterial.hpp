//
// Created by piero on 3/11/2024.
//
#pragma once
#include <memory>
#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Descriptors.hpp"

using namespace Lettuce::Core;

namespace Lettuce::X2D::Materials
{
    class IMaterial
    {
    public:
        uint32_t pushDataSize;
        float lineWidth;
        VkPrimitiveTopology topology;
        std::shared_ptr<Descriptors> descriptorsPtr;
        PipelineLayout layout;
        GraphicsPipeline pipeline;
        virtual void UpdateDescriptors();
    };
}