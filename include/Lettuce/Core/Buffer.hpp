/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_BUFFER_HPP
#define LETTUCE_CORE_BUFFER_HPP 

// standard headers
#include <optional>

namespace Lettuce::Core
{
    struct BufferCreateInfo
    {
        uint32_t size;
        VkBufferUsageFlags2 usage;
    };
    
    class Buffer
    {
    private:
        uint64_t m_address = 0;
        uint32_t m_size;
    public:
        VkDevice m_device;
        VkDeviceMemory m_memory;
        VkBuffer m_buffer;
        std::optional<VkDescriptorSetLayout> m_descriptorSetLayout;

        Buffer(VkDevice device, const BufferCreateInfo& createInfo);
        void Release();

        uint64_t GetAddress();
        uint32_t GetSize();
    };
}
#endif // LETTUCE_CORE_BUFFER_HPP