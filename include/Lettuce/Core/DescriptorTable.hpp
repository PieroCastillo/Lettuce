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
        std::string layoutName;
        std::string instanceName;
        std::vector<std::tuple<std::string, uint64_t, VkDescriptorType, VkDeviceAddress, VkDeviceSize>> bufferBindings;
        std::vector<std::tuple<std::string, uint64_t, VkDescriptorType, VkSampler*, VkImageView, VkImageLayout>> textureBindings;
    public:
        void Bind(const std::string& name, const BufferHandle& handle);
        void Bind(const std::string& name, const TextureHandle& handle, const std::optional<std::shared_ptr<Sampler>> sampler);
    };

    struct DescriptorTableCreateInfo
    {
        std::vector<DescriptorSetLayoutInfo> setLayoutInfos;
        uint64_t maxDescriptorVariantsPerSet;
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
        uint64_t m_mappedData;
        std::unordered_map<VkDescriptorType, uint64_t> m_descriptorTypeSizeMap;
        std::vector<VkDescriptorSetLayout> m_setLayouts;
        std::unordered_map<std::string, std::tuple<VkDescriptorSetLayout, DescriptorSetLayoutInfo>> m_nameLayout_LayoutInfoMap;

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
    };
}
#endif // LETTUCE_CORE_DESCRIPTOR_TABLE_HPP