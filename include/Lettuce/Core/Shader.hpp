//
// Created by piero on 6/03/2025.
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "IReleasable.hpp"
#include "Descriptors.hpp"
#include "PipelineLayout.hpp"
#include "Utils.hpp"

namespace Lettuce::Core
{
    class Shader : public IReleasable
    {
    private:
        std::vector<VkSpecializationMapEntry> entries;
        std::vector<void *> datas;
        uint32_t currentOffset = 0;

    public:
        VkShaderEXT _shader;
        std::shared_ptr<Device> _device;
        std::shared_ptr<PipelineLayout> _layout;

        Shader(const std::shared_ptr<Device> &device, const std::shared_ptr<PipelineLayout> &layout) : _device(device), _layout(layout)
        {
        }

        void Release();

        void Assemble(const VkShaderStageFlagBits &stage,
                      const std::string &entryPoint,
                      const std::vector<uint32_t> &code,
                      const VkShaderStageFlags &nextStage = 0,
                      const VkShaderCreateFlagsEXT &flags = VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);

        void AddConstant(uint32_t constantId, uint32_t size, void *data);
    };
}