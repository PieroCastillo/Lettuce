/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_DESCRIPTOR_TABLE_HPP
#define LETTUCE_CORE_DESCRIPTOR_TABLE_HPP 

namespace Lettuce::Core
{
    class DescriptorTable
    {
    private:
    
    public:
        VkDevice m_device;
        VkDeviceMemory m_descriptorBufferMemory;
        VkBuffer m_descriptorBuffer;

        DescriptorTable(VkDevice device);
        void Release();
    };
}
#endif // LETTUCE_CORE_DESCRIPTOR_TABLE_HPP