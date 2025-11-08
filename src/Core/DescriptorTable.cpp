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


void DescriptorSetInstance::Bind(const std::string& name, const BufferHandle& handle)
{

}

void DescriptorSetInstance::Bind(const std::string& name, const TextureHandle& handle, const std::optional<std::shared_ptr<Sampler>> sampler)
{

}

void DescriptorTable::Create(const IDevice& device, const DescriptorTableCreateInfo& createInfo)
{
    m_device = device.m_device;
    auto gpu = device.m_physicalDevice;
    VkPhysicalDeviceDescriptorBufferPropertiesEXT dbProps = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT,
    };
    VkPhysicalDeviceProperties2 props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &dbProps,
    };
    vkGetPhysicalDeviceProperties2(gpu, &props);

    VkDescriptorSetLayoutCreateInfo dscSetLayoutCI = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
    };

    m_bufferSize = 0;
    // per set
    for (auto setLayoutInfo : createInfo.setLayoutInfos)
    {
        std::vector<VkDescriptorSetLayoutBinding> vkBindings;
        vkBindings.reserve(setLayoutInfo.bindings.size());

        // per binding
        for (int i = 0; i < setLayoutInfo.bindings.size(); ++i)
        {
            VkDescriptorSetLayoutBinding vkBinding = {
                .binding = setLayoutInfo.bindings[i].bindingIdx,
                .descriptorType = setLayoutInfo.bindings[i].type,
                .descriptorCount = setLayoutInfo.bindings[i].count,
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

        std::println("Descriptor Set Layout: {}, Size: {}", setLayoutInfo.setIdx, dscSetLayoutSize);

        // copy descriptor set layout
        m_setLayouts.push_back(dscSetLayout);


        m_nameLayout_LayoutInfoMap[setLayoutInfo.setName] = std::make_tuple(dscSetLayout, std::move(setLayoutInfo));
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

    m_bufferSize = memReqs.size; // set buffer size
    m_currentOffset = 0;
    // set descriptor sizes
    m_bufferAlignment = dbProps.descriptorBufferOffsetAlignment;
    m_descriptorTypeSizeMap[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] = dbProps.sampledImageDescriptorSize;
    m_descriptorTypeSizeMap[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] = dbProps.storageImageDescriptorSize;
    m_descriptorTypeSizeMap[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = dbProps.uniformBufferDescriptorSize;
    m_descriptorTypeSizeMap[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] = dbProps.storageBufferDescriptorSize;
    m_descriptorTypeSizeMap[VK_DESCRIPTOR_TYPE_SAMPLER] = dbProps.samplerDescriptorSize;
    m_descriptorTypeSizeMap[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = dbProps.combinedImageSamplerDescriptorSize;
    m_descriptorTypeSizeMap[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] = dbProps.inputAttachmentDescriptorSize;

    VkMemoryAllocateInfo memoryAI = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = findMemoryTypeIndex(m_device, device.m_physicalDevice, memReqs.memoryTypeBits, MemoryAccess::FastCPUWriteGPURead),
    };
    handleResult(vkAllocateMemory(m_device, &memoryAI, nullptr, &m_descriptorBufferMemory));

    // bind memory to buffer
    handleResult(vkBindBufferMemory(m_device, m_descriptorBuffer, m_descriptorBufferMemory, 0));

    // prepare memory to be mapped
    vkMapMemory(m_device, m_descriptorBufferMemory, 0, m_bufferSize, 0, (void**)&m_mappedData);

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

    vkUnmapMemory(m_device, m_descriptorBufferMemory);
    vkDestroyBuffer(m_device, m_descriptorBuffer, nullptr);
    vkFreeMemory(m_device, m_descriptorBufferMemory, nullptr);
}

DescriptorSetInstance& DescriptorTable::CreateSetInstance(const std::string& paramsBlockName, const std::string& instanceName)
{
    auto it = m_nameLayout_LayoutInfoMap.find(paramsBlockName);
    if (it == m_nameLayout_LayoutInfoMap.end())
    {
        throw LettuceException(LettuceResult::NotFound);
    }

    const auto& [layout, layoutInfo] = it->second;

    auto instance = std::make_unique<DescriptorSetInstance>();
    instance->layoutName = paramsBlockName;
    instance->instanceName = instanceName;

    auto& ref = *instance;
    m_nameInstance_SetInstanceMap[instanceName] = std::move(instance);
    return ref;
}

void DescriptorTable::BuildSets()
{
    for (auto& [instanceName, instancePtr] : m_nameInstance_SetInstanceMap)
    {
        auto& instance = *instancePtr;
        uint64_t instanceOffset = m_currentOffset;

        for (const auto& [name, offset, descriptorType, address, size] : instance.bufferBindings)
        {
            VkDescriptorAddressInfoEXT addressInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT,
                .address = address,
                .range = size,
            };

            VkDescriptorDataEXT data;
            switch (descriptorType)
            {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                data.pUniformBuffer = &addressInfo;
                break;
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                data.pStorageBuffer = &addressInfo;
                break;
            }

            VkDescriptorGetInfoEXT getInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
                .type = descriptorType,
                .data = data,
            };

            auto dSize = align_up(m_descriptorTypeSizeMap[descriptorType], m_bufferAlignment);
            vkGetDescriptorEXT(m_device, &getInfo, dSize, (uint8_t*)m_mappedData + m_currentOffset);
            m_currentOffset += dSize;
        }

        for (const auto& [name, offset, descriptorType, samplerPtr, view, layout] : instance.textureBindings)
        {
            VkDescriptorImageInfo imageInfo = {
                .sampler = *samplerPtr,
                .imageView = view,
                .imageLayout = layout,
            };

            VkDescriptorDataEXT data;
            switch (descriptorType)
            {
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                data.pSampledImage = &imageInfo;
                break;
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                data.pStorageImage = &imageInfo;
                break;
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                data.pCombinedImageSampler = &imageInfo;
                break;
            case VK_DESCRIPTOR_TYPE_SAMPLER:
                data.pSampler = samplerPtr;
                break;
            }

            VkDescriptorGetInfoEXT getInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
                .type = descriptorType,
                .data = data,
            };

            auto dSize = align_up(m_descriptorTypeSizeMap[descriptorType], m_bufferAlignment);
            vkGetDescriptorEXT(m_device, &getInfo, dSize, (uint8_t*)m_mappedData + m_currentOffset);
            m_currentOffset += dSize;
        }

        m_nameInstance_OffsetMap[instanceName] = instanceOffset;
    }
}


void DescriptorTable::Reset()
{
    m_nameInstance_OffsetMap.clear();
    m_nameInstance_SetInstanceMap.clear();
    m_currentOffset = 0;
}