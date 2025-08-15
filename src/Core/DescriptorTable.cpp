// standard headers
#include <memory>
#include <vector>
#include <unordered_map>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/DescriptorTable.hpp"

using namespace Lettuce::Core;

DescriptorTable::DescriptorTable(VkDevice device, const DescriptorTableCreateInfo &createInfo, LettuceResult& result)
{
    // initialize descriptor buffer and its device memory
    VkMemoryAllocateInfo memoryAI = {

    };
    checkResult(vkAllocateMemory(m_device, &memoryAI, nullptr, &m_descriptorBufferMemory));

    VkBufferCreateInfo descriptorBufferCI = {

    };
    checkResult(vkCreateBuffer(m_device, &descriptorBufferCI, nullptr, &m_descriptorBuffer));
}

void DescriptorTable::Release()
{
    vkDestroyBuffer(m_device, m_descriptorBuffer, nullptr);
    vkFreeMemory(m_device, m_descriptorBufferMemory, nullptr);
}

void DescriptorTable::SetBuffer(uint32_t set, uint32_t binding, VkBuffer buffer, DescriptorAddressType addressType)
{
    VkDescriptorAddressInfoEXT addressInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT,
        .address = 0, // buffer address
        .range = 0,   // size of the buffer
        .format = VK_FORMAT_UNDEFINED,
    };

    VkDescriptorGetInfoEXT getInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
        .type = 0,
        .data = 0,
    };

    vkGetDescriptorEXT(m_device, &getInfo, dataSize, &pDescriptor);
}

void DescriptorTable::SetTexture(uint32_t set, uint32_t binding, VkImageView imageView, VkSampler sampler, DescriptorTextureType textureType)
{
    VkDescriptorType dt = {};
    VkDescriptorDataEXT data = {};

    VkDescriptorImageInfo imageInfo = {
        .sampler = sampler,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    if (textureType == DescriptorTextureType::StorageImage)
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    if (imageView != VK_NULL_HANDLE & sampler != VK_NULL_HANDLE)
    {
        dt = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        data.pCombinedImageSampler = &imageInfo;
    }
    else if (imageView == VK_NULL_HANDLE & sampler != VK_NULL_HANDLE)
    {
        dt = VK_DESCRIPTOR_TYPE_SAMPLER;
        data.pSampler = &sampler;
    }
    else if (imageView != VK_NULL_HANDLE & sampler == VK_NULL_HANDLE)
    {
        switch (textureType)
        {
        case DescriptorTextureType::SampledImage:
            dt = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            data.pSampledImage = &imageInfo;
            break;
        case DescriptorTextureType::StorageImage:
            dt = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            data.pStorageImage = &imageInfo;
            break;
        case DescriptorTextureType::InputAttachment:
            dt = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            data.pInputAttachmentImage = &imageInfo;
            break;
        default:
            break;
        }
    }

    VkDescriptorGetInfoEXT getInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
        .type = dt,
        .data = data,
    };
    // TODO: dataSize & pDescriptor
    vkGetDescriptorEXT(m_device, &getInfo, dataSize, &pDescriptor);
}