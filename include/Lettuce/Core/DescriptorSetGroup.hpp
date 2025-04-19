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
#include "DescriptorLayoutGroup.hpp"
#include "BufferResource.hpp"
#include "ImageViewResource.hpp"

namespace Lettuce::Core
{

    class DescriptorSetGroup : public IReleasable, public IManageHandleGroup<VkDescriptorSet>
    {
    private:
    public:
        std::shared_ptr<Device> device;
        std::shared_ptr<DescriptorLayoutGroup> layoutGroup;

        DescriptorSetGroup(const std::shared_ptr<Device> &device, const std::shared_ptr<DescriptorLayoutGroup> &layoutGroup)
            : device(device), layoutGroup(layoutGroup)
        {
        }

        void Release();

        void Assemble(uint32_t maxSets = 16);
    };
}