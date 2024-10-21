//
// Created by piero on 26/06/2024
//
#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <tuple>
#include "Device.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"
#include "Utils.hpp"
#include "TextureView.hpp"

namespace Lettuce::Core
{
    class Descriptors
    {
    private:
        struct WriteFieldsInfo{
            std::vector<VkDescriptorImageInfo> imageInfos;
            std::vector<VkDescriptorBufferInfo> bufferInfos;
        };
        std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindingsMap; //stores raw info about descriptors per set
        std::map<VkDescriptorType, uint32_t> typesMap; //stores how much descriptors are per descriptor type
        std::map<std::pair<uint32_t,uint32_t>, VkDescriptorType> bindingsTypes; // access by set and binding what is the descriptorType
        std::vector<VkDescriptorPoolSize> sizes;
        std::vector<VkWriteDescriptorSet> writes;
        std::map<std::pair<uint32_t,uint32_t>, WriteFieldsInfo> writesFieldsMap; //stores image and buffer infos to update

    public:
        Device _device;
        std::vector<VkDescriptorSet> _descriptorSets;
        VkDescriptorPool _pool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayout> _layouts;

        void Build(Device &device, uint32_t maxSets = 16);
        void Destroy();

        void AddBinding(uint32_t set, uint32_t binding, DescriptorType type, PipelineStage stage, uint32_t descriptorCount = 1);

        void AddUpdateInfo(uint32_t set, uint32_t binding, uint32_t size, std::vector<Buffer> buffers);
        void AddUpdateInfo(uint32_t set, uint32_t binding, std::vector<TextureView> views);
        void AddUpdateInfo(uint32_t set, uint32_t binding, std::vector<Sampler> samplers);
        void AddUpdateInfo(uint32_t set, uint32_t binding, std::vector<Sampler> samplers, std::vector<TextureView> views);

        template <typename TBufferDataType>
        void AddUpdateInfo(uint32_t set, uint32_t binding, std::vector<Buffer> buffers)
        {
            AddUpdateInfo(set, binding, (uint32_t)sizeof(TBufferDataType), buffers);
        }

        void Update();
    };
}