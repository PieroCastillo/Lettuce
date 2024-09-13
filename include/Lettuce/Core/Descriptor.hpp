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

        void AddUpdateInfo(uint32_t binding, DescriptorType type, Sampler sampler, TextureView view);

        template <typename TBufferDataType>
        void AddUpdateInfo(uint32_t binding, Buffer &buffer)
        {
            VkDescriptorBufferInfo descriptorBufferI = {
                .buffer = buffer._buffer,
                .offset = 0,
                .range = sizeof(TBufferDataType),
            };

            VkWriteDescriptorSet write = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = _descriptorSet,
                .dstBinding = binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = (VkDescriptorType)bindings[binding].descriptorType,
                .pBufferInfo = &descriptorBufferI,
            };
            writes.push_back(write);
        }

        void Update();
    };
}