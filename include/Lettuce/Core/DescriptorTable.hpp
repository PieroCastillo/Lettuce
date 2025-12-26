/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_DESCRIPTOR_TABLE_HPP
#define LETTUCE_CORE_DESCRIPTOR_TABLE_HPP

// standard headers
#include <string_view>
#include <vector>
#include <string>
#include <unordered_map>
#include <tuple>
#include <optional>
#include <utility>

// project headers
#include "common.hpp"
#include "Sampler.hpp"
#include "Texture.hpp"

namespace Lettuce::Core
{
    class DescriptorTable;

    struct DescriptorTableCreateInfo
    {
        std::vector<DescriptorSetLayoutInfo> setLayoutInfos;
        uint32_t textureDescriptorCount;
        uint32_t samplerDescriptorCount;
    };

    /*
    Descriptor Table Buffer Layout:
    |                   Allocation                   |
    |------------------------------------------------|
    |                     Buffer                     |
    | TD 1 |    ...   | TD n | SD 1 |   ...   | SD m |
    */
    class DescriptorTable
    {
    private:
        // constant params
        VkDevice m_device;
        VkDeviceMemory m_descriptorBufferMemory;
        VkBuffer m_descriptorBuffer;
        VkPipelineLayout m_pipelineLayout;

        uint64_t m_bufferSize;
        uint64_t m_bufferAlignment;
        uint32_t m_textureDescriptorCount;
        uint32_t m_samplerDescriptorCount;

        void* m_cpuAddress;
        void* m_gpuAddress;

        void* m_pushPayload;
        
        std::unordered_map<VkDescriptorType, uint64_t> m_descriptorTypeSizeMap;
        std::vector<VkDescriptorSetLayout> m_setLayouts;
        std::unordered_map<VkDescriptorSetLayout, DescriptorSetLayoutInfo> m_layout_LayoutInfoMap;

        // variable params
        uint64_t m_currentOffset;

    public:
        void Create(const IDevice& device, const DescriptorTableCreateInfo& createInfo);
        void Release();

        void Reset();
        void* GetGPUAddress();

        void Set(int index, const std::shared_ptr<Texture>& texture);
        void Set(int index, const std::shared_ptr<Sampler>& sampler);
        void Set(int index, void* gpuPtr);
    };
}
#endif // LETTUCE_CORE_DESCRIPTOR_TABLE_HPP