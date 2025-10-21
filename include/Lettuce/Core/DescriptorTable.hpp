/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_DESCRIPTOR_TABLE_HPP
#define LETTUCE_CORE_DESCRIPTOR_TABLE_HPP

// standard headers
#include <vector>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    enum class DescriptorAddressType
    {
        UniformBuffer = 6,
        StorageBuffer = 7,
        UniformTexelBuffer = 4,
        StorageTexelBuffer = 5,
    };

    enum class DescriptorTextureType
    {
        StorageImage,
        SampledImage, 
        InputAttachment,
    };

    struct DescriptorBinding
    {
        VkDescriptorType type;
        uint32_t count;
        VkShaderStageFlags stages;
    };

    struct DescriptorTableCreateInfo
    {
        std::vector<std::vector<DescriptorBinding>> bindings;
    };

    class DescriptorTable
    {
    private:
    public:
        VkDevice m_device;
        VkDeviceMemory m_descriptorBufferMemory;
        VkBuffer m_descriptorBuffer;

        void Create(const IDevice& device, const DescriptorTableCreateInfo& createInfo);
        void Release();

        void SetBuffer(uint32_t set, uint32_t binding, VkBuffer buffer, DescriptorAddressType addressType);
        void SetTexture(uint32_t set, uint32_t binding, VkImageView imageView, VkSampler sampler, DescriptorTextureType textureType);
    };
}
#endif // LETTUCE_CORE_DESCRIPTOR_TABLE_HPP