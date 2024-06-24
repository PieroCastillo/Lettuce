//
// Created by piero on 14/02/2024.
//
module;
#include <iostream>
#define VOLK_IMPLEMENTATION
#include <volk.h>

export module Lettuce:GraphicsPipeline;

import :Device;
import :Utils;
import :PipelineConnector;

export namespace Lettuce::Core
{
    class GraphicsPipeline
    {
        Device _device;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;

        void Create(Device &device, PipelineConnector &connector){
            _device = device;
            _pipelineLayout = connector._pipelineLayout;

            VkGraphicsPipelineCreateInfo graphicPipelineCI = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            };
        }
    };
}