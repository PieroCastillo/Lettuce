// standard headers
#include <array>
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
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

/*
    |-                      Allocation                     --|
    ----------------------------------------------------------
    |---                       Buffer                      --|
    |----Sampled Images -----|--Samplers--|--Storage Images--|

    The Descriptor Table have 3 binding & 1 set
*/

DescriptorTable Device::CreateDescriptorTable(const DescriptorTableDesc& desc)
{
    VkDevice device = impl->m_device;
    VkPhysicalDevice gpu = impl->m_physicalDevice;

    VkDeviceMemory memory;
    VkBuffer buffer;
    VkDescriptorSetLayout setLayout;
    VkPipelineLayout pipelineLayout;

    uint32_t sampledImageDescriptorSize = impl->props.sampledImageDescriptorSize;
    uint32_t samplerDescriptorSize = impl->props.samplerDescriptorSize;
    uint32_t storageImageDescriptorSize = impl->props.storageImageDescriptorSize;

    uint64_t sampledImagesBindingOffset;
    uint64_t samplersBindingOffset;
    uint64_t storageImagesBindingOffset;

    uint64_t bufferSize;
    std::array<VkDescriptorSetLayoutBinding, 3> bindings;
    VkMemoryRequirements memReqs = {};
    void* cpuPtr;

    // create descriptor set layout
    bindings[0] = { 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, desc.sampledImageDescriptorCount, VK_SHADER_STAGE_ALL, nullptr };
    bindings[1] = { 1, VK_DESCRIPTOR_TYPE_SAMPLER, desc.samplerDescriptorCount, VK_SHADER_STAGE_ALL, nullptr };
    bindings[2] = { 2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, desc.storageImageDescriptorCount, VK_SHADER_STAGE_ALL, nullptr };

    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
        .bindingCount = (uint32_t)bindings.size(),
        .pBindings = bindings.data(),
    };
    handleResult(vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, nullptr, &setLayout));

    vkGetDescriptorSetLayoutBindingOffsetEXT(device, setLayout, 0, &sampledImagesBindingOffset);
    vkGetDescriptorSetLayoutBindingOffsetEXT(device, setLayout, 1, &samplersBindingOffset);
    vkGetDescriptorSetLayoutBindingOffsetEXT(device, setLayout, 2, &storageImagesBindingOffset);

    vkGetDescriptorSetLayoutSizeEXT(device, setLayout, &bufferSize);

    // create buffer & device memory
    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    handleResult(vkCreateBuffer(device, &bufferCI, nullptr, &buffer));

    vkGetBufferMemoryRequirements(device, buffer, &memReqs);

    // MemoryAccess must be Shared, because we need the cpu to access gpu memory
    VkMemoryAllocateInfo memAlloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = findMemoryTypeIndex(device, gpu, memReqs.memoryTypeBits, MemoryAccess::Shared),
    };
    handleResult(vkAllocateMemory(device, &memAlloc, nullptr, &memory));
    handleResult(vkBindBufferMemory(device, buffer, memory, 0));

    handleResult(vkMapMemory(device, memory, 0, bufferSize, 0, &cpuPtr));

    // get buffer device address
    VkBufferDeviceAddressInfo addressInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer,
    };
    uint64_t  gpuPtr = vkGetBufferDeviceAddress(device, &addressInfo);

    // create pipeline layout
    // TODO: impl bounds checking
    VkPushConstantRange pushConstant = {
        .stageFlags = VK_SHADER_STAGE_ALL,
        .offset = 0,
        .size = 128,
    };

    VkPipelineLayoutCreateInfo pipelineLayoutCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &setLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstant,
    };
    handleResult(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout));

    void* pushPayloadPtr = malloc(128);

    return impl->descriptorTables.allocate({
        .descriptorBufferMemory = memory,
        .descriptorBuffer = buffer,
        .setLayout = setLayout,
        .pipelineLayout = pipelineLayout,
        .bufferSize = bufferSize,
        .sampledImageDescriptorCount = desc.sampledImageDescriptorCount,
        .samplerDescriptorCount = desc.samplerDescriptorCount,
        .storageImageDescriptorCount = desc.storageImageDescriptorCount,
        .sampledImagesBindingOffset = sampledImagesBindingOffset,
        .samplersBindingOffset = samplersBindingOffset,
        .storageImagesBindingOffset = storageImagesBindingOffset,
        .cpuAddress = (uint64_t*)cpuPtr,
        .gpuAddress = gpuPtr,
        .pushPayloadSize = 128,
        .pushPayloadAddress = (uint64_t*)pushPayloadPtr,
        });
}

void Device::Destroy(DescriptorTable descriptorTable)
{
    VkDevice device = impl->m_device;
    auto& dt = impl->descriptorTables.get(descriptorTable);

    free(dt.pushPayloadAddress);
    vkDestroyPipelineLayout(device, dt.pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, dt.setLayout, nullptr);
    vkUnmapMemory(device, dt.descriptorBufferMemory);
    vkDestroyBuffer(device, dt.descriptorBuffer, nullptr);
    vkFreeMemory(device, dt.descriptorBufferMemory, nullptr);

    impl->descriptorTables.free(descriptorTable);
}

void Device::PushResourceDescriptors(
    DescriptorTable descriptorTable,
    std::span<const std::pair<uint32_t, Texture>> sampledImages,
    std::span<const std::pair<uint32_t, Sampler>> samplers,
    std::span<const std::pair<uint32_t, Texture>> storageImages
)
{
    auto& dt = impl->descriptorTables.get(descriptorTable);

    VkDevice device = impl->m_device;
    uint32_t sampledImageDescriptorSize = impl->props.sampledImageDescriptorSize;
    uint32_t samplerDescriptorSize = impl->props.samplerDescriptorSize;
    uint32_t storageImageDescriptorSize = impl->props.storageImageDescriptorSize;
    void* descriptor = nullptr;

    for (auto& [idx, imgHandle] : sampledImages)
    {
        auto& img = impl->textures.get(imgHandle);

        VkDescriptorImageInfo imgInfo = { .imageView = img.imageView };

        VkDescriptorGetInfoEXT getInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .pNext = nullptr,
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .data = {.pSampledImage = &imgInfo},
        };
        // TODO: impl bounds checking
        uint64_t offset = dt.sampledImagesBindingOffset + (idx * sampledImageDescriptorSize);
        vkGetDescriptorEXT(device, &getInfo, sampledImageDescriptorSize, (uint64_t*)(dt.cpuAddress + offset));
    }

    for (auto& [idx, samplerHandle] : samplers)
    {
        auto& sampler = impl->samplers.get(samplerHandle);

        VkDescriptorGetInfoEXT getInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .pNext = nullptr,
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .data = {.pSampler = &sampler},
        };
        // TODO: impl bounds checking
        uint64_t offset = dt.samplersBindingOffset + (idx * samplerDescriptorSize);
        vkGetDescriptorEXT(device, &getInfo, samplerDescriptorSize, (uint64_t*)(dt.cpuAddress + offset));
    }

    for (auto& [idx, imgHandle] : storageImages)
    {
        auto& img = impl->textures.get(imgHandle);

        VkDescriptorImageInfo imgInfo = { .imageView = img.imageView };

        VkDescriptorGetInfoEXT getInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .pNext = nullptr,
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .data = {.pStorageImage = &imgInfo},
        };
        // TODO: impl bounds checking
        uint64_t offset = dt.storageImagesBindingOffset + (idx * storageImageDescriptorSize);
        vkGetDescriptorEXT(device, &getInfo, sampledImageDescriptorSize, (uint64_t*)(dt.cpuAddress + offset));
    }

    // TODO: in the future, enable acceleration structures
}

void Device::PushAllocations(DescriptorTable descriptorTable, std::span<const std::pair<uint32_t, const MemoryView&>> allocations)
{
    auto& dt = impl->descriptorTables.get(descriptorTable);
    auto* dstAddress = dt.pushPayloadAddress;

    // at least 16 buffers (because 128 bytes is the minimum)
    // TODO: impl 256 bytes for push constant
    for (const auto& [idx, alloc] : allocations)
    {
        if (idx > 16)
            continue;

        dstAddress[idx] = alloc.gpuAddress;
    }
}