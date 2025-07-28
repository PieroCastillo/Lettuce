/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_DESCRIPTOR_TABLE_HPP
#define LETTUCE_CORE_DESCRIPTOR_TABLE_HPP

namespace Lettuce::Core
{
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

        DescriptorTable(VkDevice device, const DescriptorTableCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_DESCRIPTOR_TABLE_HPP