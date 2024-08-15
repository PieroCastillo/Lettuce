//
// Created by piero on 23/06/2024.
//
#include <iostream>
#include <vector>
#include <volk.h>

namespace Lettuce::Core
{
    class PipelineConnector
    {
    public:
        Device _device;
        VkPipelineLayout _pipelineLayout;
        std::vector<VkPushConstantRange> pushConstants;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

        void AddDescriptor(DescriptorLayout &descriptor)
        {
            descriptorSetLayouts.emplace_back(descriptor._setLayout);
        }

        template <typename T1>
        void AddPushConstant(uint32_t offset, PipelineStage stage)
        {
            VkPushConstantRange pushConstantRange = {
                .stageFlags = (VkShaderStageFlags)stage,
                .offset = offset,
                .size = sizeof(T1),
            };
            pushConstants.emplace_back(pushConstantRange);
        }

        void Build(Device &device)
        {
            _device = device;

            VkPipelineLayoutCreateInfo pipelineLayoutCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .flags = VkPipelineLayoutCreateFlagBits::VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT
            };

            if (pushConstants.size() > 0)
            {
                pipelineLayoutCI.pPushConstantRanges = pushConstants.data();
                pipelineLayoutCI.pushConstantRangeCount = (uint32_t)pushConstants.size();
            }

            if (descriptorSetLayouts.size() > 0)
            {
                pipelineLayoutCI.pSetLayouts = descriptorSetLayouts.data();
                pipelineLayoutCI.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
            }

            checkResult(vkCreatePipelineLayout(_device._device, &pipelineLayoutCI, nullptr, &_pipelineLayout), "PipelineLayout created sucessfully");
        }

        void Destroy()
        {
            vkDestroyPipelineLayout(_device._device, _pipelineLayout, nullptr);
        }
    };
}