//
// Created by piero on 26/06/2024
//
#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <tuple>
#include <memory>
#include "Device.hpp"
#include "IReleasable.hpp"
#include "Sampler.hpp"
#include "Utils.hpp"
#include "BufferResource.hpp"
#include "ImageViewResource.hpp"

namespace Lettuce::Core
{
    class DescriptorLayoutGroup : public IReleasable, public IManageHandleGroup<VkDescriptorSetLayout>
    {
    private:
        std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindings;
        bool allowDescriptorBuffers = false;

    public:
        std::shared_ptr<Device> device;

        void AddBinding(uint32_t set,
                        uint32_t binding,
                        VkDescriptorType type,
                        VkShaderStageFlags stages,
                        uint32_t descriptorCount);

        void Assemble();

        DescriptorLayoutGroup(const std::shared_ptr<Device> &device, bool allowDescriptorBuffers = false) : device(device),
                                                                                                  allowDescriptorBuffers(allowDescriptorBuffers) {}
        void Release();
    };
}