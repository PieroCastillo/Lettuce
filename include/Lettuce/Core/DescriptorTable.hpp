/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_DESCRIPTOR_TABLE_HPP
#define LETTUCE_CORE_DESCRIPTOR_TABLE_HPP

// standard headers
#include <string_view>
#include <vector>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    struct DescriptorTableCreateInfo
    {
        std::vector<DescriptorBindingsInfo> bindings;
        uint64_t maxDescriptorVariantsPerSet;
    };

    /*
    Descriptor Table Buffer Layout:
    |                 Allocation                |
    |-------------------------------------------|
    |                  Buffer                   |
    | Set0[max]  |  Set1[max] | ... | SetN[max] |
    */
    class DescriptorTable
    {
    private:
        uint32_t m_bufferSize;
    public:
        VkDevice m_device;
        VkDeviceMemory m_descriptorBufferMemory;
        VkBuffer m_descriptorBuffer;
        VkPipelineLayout m_pipelineLayout;
        std::vector<VkDescriptorSetLayout> m_setLayouts;

        void Create(const IDevice& device, const DescriptorTableCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_DESCRIPTOR_TABLE_HPP