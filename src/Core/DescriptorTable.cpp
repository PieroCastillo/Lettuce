// standard headers
#include <memory>
#include <print>
#include <unordered_map>
#include <vector>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/DescriptorTable.hpp"

using namespace Lettuce::Core;

void DescriptorTable::Create(const IDevice& device, const DescriptorTableCreateInfo& createInfo)
{
    m_device = device.m_device;

    VkDescriptorSetLayoutCreateInfo dscSetLayoutCI = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
    };

    m_bufferSize = 0;
    // per set
    for (const auto& setBindings : createInfo.bindings)
    {
        std::vector<VkDescriptorSetLayoutBinding> vkBindings;
        vkBindings.reserve(setBindings.counts.size());

        // per binding
        for (int i = 0; i < setBindings.bindingId.size(); ++i)
        {
            VkDescriptorSetLayoutBinding vkBinding = {
                .binding = setBindings.bindingId[i],
                .descriptorType = setBindings.types[i],
                .descriptorCount = setBindings.counts[i],
                .stageFlags = VK_SHADER_STAGE_ALL,
            };
            vkBindings.push_back(vkBinding);
        }

        dscSetLayoutCI.bindingCount = (uint32_t)vkBindings.size();
        dscSetLayoutCI.pBindings = vkBindings.data();

        // create descriptor set layout
        VkDescriptorSetLayout dscSetLayout;
        handleResult(vkCreateDescriptorSetLayout(m_device, &dscSetLayoutCI, nullptr, &dscSetLayout));

        uint64_t dscSetLayoutSize = 0;
        vkGetDescriptorSetLayoutSizeEXT(m_device, dscSetLayout, (VkDeviceSize*)(&dscSetLayoutSize));
        m_bufferSize += dscSetLayoutSize;

        std::println("Descriptor Set Layout: {}, Size: {}", setBindings.setId, dscSetLayoutSize);

        // copy descriptor set layout
        m_setLayouts.push_back(dscSetLayout);
    }
    m_bufferSize *= createInfo.maxDescriptorVariantsPerSet;

    // initialize descriptor buffer and its device memory

    VkBufferCreateInfo descriptorBufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = m_bufferSize,
        .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    handleResult(vkCreateBuffer(m_device, &descriptorBufferCI, nullptr, &m_descriptorBuffer));

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_device, m_descriptorBuffer, &memReqs);

    VkMemoryAllocateInfo memoryAI = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = findMemoryTypeIndex(m_device, device.m_physicalDevice, memReqs.memoryTypeBits, MemoryAccess::FastCPUWriteGPURead),
    };
    handleResult(vkAllocateMemory(m_device, &memoryAI, nullptr, &m_descriptorBufferMemory));

    // bind memory to buffer
    handleResult(vkBindBufferMemory(m_device, m_descriptorBuffer, m_descriptorBufferMemory, 0));

    // create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = (uint32_t)m_setLayouts.size(),
        .pSetLayouts = m_setLayouts.data(),
        // .pPushConstantRanges  | TODO : use push constants for pointers to buffers, 
    };
    handleResult(vkCreatePipelineLayout(m_device, &pipelineLayoutCI, nullptr, &m_pipelineLayout));
}

void DescriptorTable::Release()
{
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);

    for (const auto& setLayout : m_setLayouts)
    {
        vkDestroyDescriptorSetLayout(m_device, setLayout, nullptr);
    }

    vkDestroyBuffer(m_device, m_descriptorBuffer, nullptr);
    vkFreeMemory(m_device, m_descriptorBufferMemory, nullptr);
}