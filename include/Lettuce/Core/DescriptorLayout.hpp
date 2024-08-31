//
// Created by piero on 4/08/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include "Device.hpp"
#include "Descriptor.hpp"

namespace Lettuce::Core
{
    class DescriptorLayout
    {
    public:
        Device _device;
        VkDescriptorSetLayout _setLayout;
        VkDescriptorPool _descriptorPool;
        uint32_t maxDescriptorSets;
        uint32_t maxDescriptorCount;
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::set<VkDescriptorType> usedTypes;
        std::vector<VkDescriptorSet> _descriptorSets;

        void AddDescriptorBinding(uint32_t binding,
                                  DescriptorType type,
                                  PipelineStage stage);

        void Build(Device &device, std::vector<uint32_t> descriptorCounts, uint32_t maxDescriptorSets = 1, uint32_t maxDescriptorCount = 1);

        void BuildPool();

        void BuildInternal(std::vector<uint32_t> descriptorCounts);

        std::vector<Descriptor> GetDescriptors();

        void Reset();

        void Destroy();
    };
}