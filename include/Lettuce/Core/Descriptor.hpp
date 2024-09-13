//
// Created by piero on 26/06/2024
//
#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "Device.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"
#include "Utils.hpp"
#include "TextureView.hpp"

namespace Lettuce::Core
{
    class Descriptor
    {
    private:
        std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindingsMap;
        std::map<VkDescriptorType, uint32_t> typesMap;
        std::vector<VkDescriptorPoolSize> sizes;
        std::vector<VkWriteDescriptorSet> writes;

    public:
        Device _device;
        VkDescriptorSet _descriptorSet;
        VkDescriptorPool _pool;
        std::vector<VkDescriptorSetLayout> _layouts;
        void Build(Device &device, uint32_t maxSets = 16);
        void Destroy();

        void AddBinding(uint32_t set, uint32_t binding, DescriptorType type, PipelineStage stage, uint32_t maxBindings);

        void AddUpdateInfo(uint32_t binding, uint32_t size, std::vector<Buffer> buffers);
        void AddUpdateInfo(uint32_t binding, std::vector<TextureView> views);
        void AddUpdateInfo(uint32_t binding, std::vector<Sampler> samplers);
        void AddUpdateInfo(uint32_t binding, std::vector<Sampler> samplers, std::vector<TextureView> views);

        template <typename TBufferDataType>
        void AddUpdateInfo(uint32_t binding, Buffer &buffer)
        {
            AddUpdateInfo(binding, sizeof(TBufferDataType), buffer);
        }

        void Update();
    };
}