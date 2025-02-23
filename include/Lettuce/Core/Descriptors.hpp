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
#include "Sampler.hpp"
#include "Utils.hpp"
#include "BufferResource.hpp"
#include "ImageViewResource.hpp"

namespace Lettuce::Core
{
    struct DescriptorBinding
    {
        uint32_t set;
        uint32_t binding;
        DescriptorType type;
        PipelineStage stage;
        uint32_t descriptorCount;
    };

    class Descriptors
    {
    private:
        struct WriteFieldsInfo
        {
            std::vector<VkDescriptorImageInfo> imageInfos;
            std::vector<VkDescriptorBufferInfo> bufferInfos;
        };
        std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindingsMap; // stores raw info about descriptors per set
        std::map<VkDescriptorType, uint32_t> typesMap;                             // stores how much descriptors are per descriptor type
        std::map<std::pair<uint32_t, uint32_t>, VkDescriptorType> bindingsTypes;   // access by set and binding what is the descriptorType
        std::vector<VkDescriptorPoolSize> sizes;
        std::vector<VkWriteDescriptorSet> writes;
        std::map<std::pair<uint32_t, uint32_t>, WriteFieldsInfo> writesFieldsMap; // stores image and buffer infos to update

    public:
        std::shared_ptr<Device> _device;
        std::vector<VkDescriptorSet> _descriptorSets;
        VkDescriptorPool _pool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayout> _layouts;

        void AddBinding(uint32_t set,
                        uint32_t binding,
                        DescriptorType type,
                        PipelineStage stage,
                        uint32_t descriptorCount);

        Descriptors(const std::shared_ptr<Device> &device);
        ~Descriptors();

        void Assemble(uint32_t maxSets = 16);

        void AddUpdateInfo(uint32_t set, uint32_t binding, const std::vector<std::pair<uint32_t, std::shared_ptr<BufferResource>>> &sizeBuffersPairs);
        void AddUpdateInfo(uint32_t set, uint32_t binding, const std::vector<std::shared_ptr<Sampler>> &samplers);
        void AddUpdateInfo(uint32_t set, uint32_t binding, const std::vector<std::pair<std::shared_ptr<Sampler>, std::shared_ptr<ImageViewResource>>> &samplerViewsPairs);

        void Update();
    };
}