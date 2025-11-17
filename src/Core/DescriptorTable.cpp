// standard headers
#include <memory>
#include <print>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <ranges>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/DescriptorTable.hpp"

using namespace Lettuce::Core;

std::string prefixOrAdd(std::string_view prefix, std::string_view add) {
    auto pos = prefix.find('.');
    return pos != prefix.npos ? std::string{ prefix.substr(0,pos) }
    : std::string{ prefix } + std::string{ add };
}

void DescriptorSetInstance::Register(const std::string& name, const std::vector<BufferHandle>& handles)
{
    auto [addresses, sizes] = unpack(handles, &BufferHandle::address, &BufferHandle::size);
    bufferBindings.emplace_back(name, addresses, sizes);
}

void DescriptorSetInstance::Register(const std::string& name, const std::vector<TextureHandle>& handles)
{
    auto [samplerPtrs, views, layouts] = unpack(handles, &TextureHandle::samplerPtr, &TextureHandle::view, &TextureHandle::layout);
    textureBindings.emplace_back(name, samplerPtrs, views, layouts);
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
                .descriptorCount = setLayoutInfo.bindings[i].count == 0 ? createInfo.defaultDescriptorCount : setLayoutInfo.bindings[i].count,
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

        m_nameLayout_LayoutMap[prefixOrAdd(setLayoutInfo.setName, "Set")] = dscSetLayout;
        m_layout_LayoutInfoMap[dscSetLayout] = setLayoutInfo;
    }
    m_bufferSize *= createInfo.maxDescriptorVariantsPerSet;
    m_bufferSize = (std::max)((uint64_t)16, m_bufferSize); // if there's no descriptor sets
    // initialize descriptor buffer and its device memory

    VkBufferCreateInfo descriptorBufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = m_bufferSize,
        .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                 VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
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

    VkMemoryAllocateFlagsInfo memoryFlags = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        .deviceMask = 0,
    };

    VkMemoryAllocateInfo memoryAI = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &memoryFlags,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = findMemoryTypeIndex(m_device, device.m_physicalDevice, memReqs.memoryTypeBits, MemoryAccess::FastCPUWriteGPURead),
    };
    handleResult(vkAllocateMemory(m_device, &memoryAI, nullptr, &m_descriptorBufferMemory));

    // bind memory to buffer
    handleResult(vkBindBufferMemory(m_device, m_descriptorBuffer, m_descriptorBufferMemory, 0));

    // prepare memory to be mapped
    vkMapMemory(m_device, m_descriptorBufferMemory, 0, m_bufferSize, 0, (void**)&m_mappedData);

    // get address
    VkBufferDeviceAddressInfo addressInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = m_descriptorBuffer,
    };
    m_bufferAddress = vkGetBufferDeviceAddress(m_device, &addressInfo);

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

    m_mappedData = nullptr;
    m_currentOffset = 0;
    m_descriptorTypeSizeMap.clear();
    m_setLayouts.clear();
    m_layout_LayoutInfoMap.clear();
    m_nameLayout_LayoutMap.clear();
    m_nameInstance_SetInstanceMap.clear();
    m_nameInstance_OffsetMap.clear();
}

DescriptorSetInstance& DescriptorTable::CreateSetInstance(const std::string& layoutName, const std::string& instanceName)
{
    auto it = m_nameLayout_LayoutMap.find(layoutName);

    if (it == m_nameLayout_LayoutMap.end())
    {
        throw LettuceException(LettuceResult::NotFound);
    }

    auto layout = it->second;
    auto layoutInfo = m_layout_LayoutInfoMap[layout];

    uint64_t layoutSize;
    vkGetDescriptorSetLayoutSizeEXT(m_device, layout, &layoutSize);

    auto instance = std::make_unique<DescriptorSetInstance>();
    instance->layout = layout;
    instance->instanceName = instanceName;

    auto& ref = *instance;
    m_nameInstance_SetInstanceMap[instanceName] = std::move(instance);
    return ref;
}

void DescriptorTable::BuildSets()
{
    // within the same descriptor set
    for (auto& [instanceName, instancePtr] : m_nameInstance_SetInstanceMap)
    {
        // get descriptor set instance
        auto& instance = *instancePtr;
        uint64_t instanceOffset = m_currentOffset;

        // next, get descriptor set layout information
        auto layout = instance.layout;
        auto& layoutInfo = m_layout_LayoutInfoMap[layout];

        // now we can get layout size
        uint64_t layoutSize;
        vkGetDescriptorSetLayoutSizeEXT(m_device, layout, &layoutSize);

        // iterate over bindings
        // next get binding info
        for (const auto& [name, addresses, sizes] : instance.bufferBindings)
        {
            auto bindingInfo = std::ranges::find_if(layoutInfo.bindings, [&](const auto& b) { return b.bindingName == name; });
            auto descriptorType = bindingInfo->type;
            auto bindingIdx = bindingInfo->bindingIdx;

            auto dSize = m_descriptorTypeSizeMap[descriptorType];

            uint64_t offset;
            vkGetDescriptorSetLayoutBindingOffsetEXT(m_device, layout, bindingIdx, &offset);

            for (int i = 0; i < addresses.size(); ++i)
            {
                auto address = addresses[i];
                auto size = sizes[i];

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

                vkGetDescriptorEXT(m_device, &getInfo, dSize, ((uint8_t*)m_mappedData + instanceOffset) + offset + (i * dSize));
            }
        }

        //     for (const auto& [name, samplerPtrs, views, layouts] : instance.textureBindings)
        //     {
        //         uint64_t binding;
        //         uint64_t offset;
        //         VkDescriptorType descriptorType;

        //         vkGetDescriptorSetLayoutBindingOffsetEXT(m_device, instance.m_layout, binding, &offset);

        //         for (int i = 0; i < views.size(); ++i)
        //         {
        //             VkDescriptorImageInfo imageInfo = {
        //              .sampler = *samplerPtrs[i],
        //              .imageView = views[i],
        //              .imageLayout = layouts[i],
        //             };

        //             VkDescriptorDataEXT data;
        //             switch (descriptorType)
        //             {
        //             case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        //                 data.pSampledImage = &imageInfo;
        //                 break;
        //             case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        //                 data.pStorageImage = &imageInfo;
        //                 break;
        //             case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        //                 data.pCombinedImageSampler = &imageInfo;
        //                 break;
        //             case VK_DESCRIPTOR_TYPE_SAMPLER:
        //                 data.pSampler = samplerPtrs[i];
        //                 break;
        //             }

        //             VkDescriptorGetInfoEXT getInfo = {
        //                 .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
        //                 .type = descriptorType,
        //                 .data = data,
        //             };

        //             auto dSize = align_up(m_descriptorTypeSizeMap[descriptorType], m_bufferAlignment);
        //             vkGetDescriptorEXT(m_device, &getInfo, dSize, (uint8_t*)m_mappedData + instanceOffset + offset);
        //         }
        //     }

        m_nameInstance_OffsetMap[instanceName] = instanceOffset;
        m_currentOffset += layoutSize;
    }
}

void DescriptorTable::Reset()
{
    m_nameInstance_OffsetMap.clear();
    m_nameInstance_SetInstanceMap.clear();
    m_currentOffset = 0;
}

uint64_t DescriptorTable::GetAddress()
{
    return m_bufferAddress;
}

uint32_t DescriptorTable::GetDescriptorSetLayoutCount()
{
    return (uint32_t)m_setLayouts.size();
}

uint64_t DescriptorTable::GetInstanceOffset(const std::string& instanceName)
{
    return m_nameInstance_OffsetMap[instanceName];
}