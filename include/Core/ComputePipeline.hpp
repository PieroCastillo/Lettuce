//
// Created by piero on 14/02/2024.
//
#include <iostream>
#include <volk.h>

namespace Lettuce::Core
{
    class ComputePipeline
    {
    public:
        Device _device;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;

        void Build(Device &device, PipelineConnector &connector, Swapchain &swapchain, Shader &shader);

        void Destroy();
    };
}