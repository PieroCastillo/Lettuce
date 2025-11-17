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
#include "ResourceDictionary.hpp"

namespace Lettuce::Core
{
    class DescriptorTable;

    struct DescriptorSetInstance
    {
    private:
        friend DescriptorTable;
        VkDescriptorSetLayout layout;
        std::string instanceName;
        std::vector<std::tuple<std::string, std::vector<VkDeviceAddress>, std::vector<uint32_t>>> bufferBindings;
        std::vector<std::tuple<std::string, std::vector<VkSampler*>, std::vector<VkImageView>, std::vector<VkImageLayout>>> textureBindings;
    public:
        void Register(const std::string& name, const std::vector<BufferHandle>& handle);
        void Register(const std::string& name, const std::vector<TextureHandle>& handle);
    };

    struct DescriptorTableCreateInfo
    {
        std::vector<DescriptorSetLayoutInfo> setLayoutInfos;
        uint64_t maxDescriptorVariantsPerSet;
        uint32_t defaultDescriptorCount;
    };

    /*
    Descriptor Table Buffer Layout:
    |                   Allocation                   |
    |------------------------------------------------|
    |                     Buffer                     |
    | DS 1-1 | DS 2-1 | DS 1-2 | DS 3-1 | DS 2-2 ... |
    */
    class DescriptorTable
    {
    private:
        // constant params
        uint64_t m_bufferAddress;
        uint64_t m_bufferSize;
        uint64_t m_bufferAlignment;
        void* m_mappedData;
        std::unordered_map<VkDescriptorType, uint64_t> m_descriptorTypeSizeMap;
        std::vector<VkDescriptorSetLayout> m_setLayouts;
        std::unordered_map<VkDescriptorSetLayout, DescriptorSetLayoutInfo> m_layout_LayoutInfoMap;
        std::unordered_map<std::string, VkDescriptorSetLayout> m_nameLayout_LayoutMap;

        // variable params
        uint64_t m_currentOffset;

        // maps
        std::unordered_map<std::string, std::unique_ptr<DescriptorSetInstance>> m_nameInstance_SetInstanceMap;
        std::unordered_map<std::string, uint64_t> m_nameInstance_OffsetMap;

    public:
        VkDevice m_device;
        VkDeviceMemory m_descriptorBufferMemory;
        VkBuffer m_descriptorBuffer;
        VkPipelineLayout m_pipelineLayout;

        void Create(const IDevice& device, const DescriptorTableCreateInfo& createInfo);
        void Release();

        DescriptorSetInstance& CreateSetInstance(const std::string& paramsBlockName, const std::string& instanceName);
        void BuildSets();

        void Reset();
        uint64_t GetAddress();
        uint32_t GetDescriptorSetLayoutCount();

        uint64_t GetInstanceOffset(const std::string& instanceName);
    };
}
#endif // LETTUCE_CORE_DESCRIPTOR_TABLE_HPP