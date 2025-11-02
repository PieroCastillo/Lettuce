// standard headers
#include <algorithm>

// project headers
#include "Lettuce/Core/Mesh.hpp"

// external headers
#include <fastgltf/tools.hpp>

using namespace Lettuce::Core;

struct LinearAllocator
{
    std::vector<uint8_t> buffer;
    size_t offset = 0;

    explicit LinearAllocator(size_t initialSize = 1 << 20) // 1 MB
    {
        buffer.resize(initialSize);
    }

    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t))
    {
        size_t alignedOffset = (offset + alignment - 1) & ~(alignment - 1);
        if (alignedOffset + size > buffer.size())
            buffer.resize(std::max(buffer.size() * 2, alignedOffset + size));

        void* ptr = buffer.data() + alignedOffset;
        offset = alignedOffset + size;
        return ptr;
    }

    size_t getOffset(const void* ptr) const
    {
        return static_cast<const uint8_t*>(ptr) - buffer.data();
    }

    void reset() { offset = 0; }
};

void MeshPool::setupMeshBufferMemory(VkDeviceMemory* memoryPtr, VkBuffer* bufferPtr, bool isStaging)
{
    // m_size must be set
    VkBufferCreateInfo bufferCI = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = m_size,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (isStaging)
    {
        bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }

    handleResult(vkCreateBuffer(m_device, &bufferCI, nullptr, bufferPtr));

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_device, *bufferPtr, &memReqs);

    VkMemoryAllocateInfo memoryAI =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = m_size,
        .memoryTypeIndex = findMemoryTypeIndex(m_device, m_gpu, memReqs.memoryTypeBits, MemoryAccess::FastGPUReadWrite),
    };

    if (isStaging)
    {
        memoryAI.memoryTypeIndex = findMemoryTypeIndex(m_device, m_gpu, memReqs.memoryTypeBits, MemoryAccess::FastCPUReadWrite);
    }

    handleResult(vkAllocateMemory(m_device, &memoryAI, nullptr, memoryPtr));

    handleResult(vkBindBufferMemory(m_device, *bufferPtr, *memoryPtr, 0));
}

void MeshPool::Create(const IDevice& device, const MeshPoolCreateInfo& createInfo)
{
    m_device = device.m_device;
    m_gpu = device.m_physicalDevice;
    m_transferQueue = device.m_transferQueue;
    m_transferQueueFamilyIndex = device.m_transferQueueFamilyIndex;

    VkCommandPoolCreateInfo poolCI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = m_transferQueueFamilyIndex,
    };
    handleResult(vkCreateCommandPool(m_device, &poolCI, nullptr, &m_cmdPool));

    VkCommandBufferAllocateInfo cmdAI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_cmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    handleResult(vkAllocateCommandBuffers(m_device, &cmdAI, &m_cmdBuffer));

    VkFenceCreateInfo fenceCI = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    handleResult(vkCreateFence(m_device, &fenceCI, nullptr, &m_fence));
}

void MeshPool::Release()
{
    vkDestroyFence(m_device, m_fence, nullptr);

    vkFreeCommandBuffers(m_device, m_cmdPool, 1, &m_cmdBuffer);
    vkDestroyCommandPool(m_device, m_cmdPool, nullptr);

    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_memory, nullptr);
}

void MeshPool::Load(const fastgltf::Asset& asset)
{
    LinearAllocator allocator;

    for (const auto& mesh : asset.meshes)
    {
        PrimitiveDatas datas;
        for (const auto& prim : mesh.primitives)
        {
            // find attributes
            auto* posIt = prim.findAttribute("POSITION"); // vec3<float>
            auto* norIt = prim.findAttribute("NORMAL");   // vec3<float>
            auto* tanIt = prim.findAttribute("TANGET");   // vec4<float>
            // auto* tex0It = prim.findAttribute("TEXCOORD_0"); // vec2<float or ubyte or ushort>

            // get accessors
            auto& posA = asset.accessors[posIt->accessorIndex];
            auto& norA = asset.accessors[norIt->accessorIndex];
            auto& tanA = asset.accessors[tanIt->accessorIndex];
            auto& idxA = asset.accessors[prim.indicesAccessor.value()];

            // copy to pointers
            uint32_t posASize = posA.count * 3 * sizeof(float);
            auto* posPtr = (fastgltf::math::fvec3*)allocator.allocate(posASize, alignof(fastgltf::math::fvec3));
            fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, posA, posPtr);
            datas.posOffset.push_back(allocator.getOffset(posPtr));
            uint32_t primSize = posASize;

            if (norA.bufferViewIndex.has_value())
            {
                uint32_t norASize = norA.count * 3 * sizeof(float);
                auto* norPtr = (fastgltf::math::fvec3*)allocator.allocate(norASize, alignof(fastgltf::math::fvec3));
                fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, norA, norPtr);
                datas.norOffset.push_back(allocator.getOffset(norPtr));
                primSize += norASize;
            }

            if (tanA.bufferViewIndex.has_value())
            {
                uint32_t tanASize = tanA.count * 4 * sizeof(float);
                auto* tanPtr = (fastgltf::math::fvec4*)allocator.allocate(tanASize, alignof(fastgltf::math::fvec4));
                fastgltf::copyFromAccessor<fastgltf::math::fvec4>(asset, tanA, tanPtr);
                datas.tanOffset.push_back(allocator.getOffset(tanPtr));
                primSize += tanASize;
            }

            if (idxA.bufferViewIndex.has_value())
            {
                uint32_t idxASize = idxA.count * sizeof(uint32_t);
                auto* idxPtr = (uint32_t*)allocator.allocate(idxASize, alignof(uint32_t));
                fastgltf::copyFromAccessor<uint32_t>(asset, idxA, idxPtr);
                datas.idxOffset.push_back(allocator.getOffset(idxPtr));
                primSize += idxASize;
            }

            datas.primitiveMemorySize.push_back(primSize);
            datas.vertexCount.push_back(posA.count);
            datas.indexCount.push_back(idxA.count);
        }
        m_primitiveDatas.push_back(datas);
        m_names.emplace_back(mesh.name);
    }
    m_size = allocator.offset;

    // copy to one VkDeviceMemory
    VkDeviceMemory tempMemory;
    VkBuffer tempBuffer;
    setupMeshBufferMemory(&tempMemory, &tempBuffer, true);

    // map, copy, unmap
    void* tempDataPtr;
    vkMapMemory(m_device, tempMemory, 0, VK_WHOLE_SIZE, 0, &tempDataPtr);
    memcpy(tempDataPtr, allocator.buffer.data(), m_size);
    vkUnmapMemory(m_device, tempMemory);

    // create on-gpu buffer
    setupMeshBufferMemory(&m_memory, &m_buffer, false);

    vkResetFences(m_device, 1, &m_fence);
    // prepare transfer

    // START RECORDING
    VkCommandBufferBeginInfo beginI = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    handleResult(vkBeginCommandBuffer(m_cmdBuffer, &beginI));

    VkBufferCopy region = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = m_size,
    };
    vkCmdCopyBuffer(m_cmdBuffer, tempBuffer, m_buffer, 1, &region);

    handleResult(vkEndCommandBuffer(m_cmdBuffer));
    // END RECORDING

    VkSubmitInfo submitI = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_cmdBuffer,
    };
    // send to transfer queue & wait for it
    handleResult(vkQueueSubmit(m_transferQueue, 1, &submitI, m_fence));
    handleResult(vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, (std::numeric_limits<uint64_t>::max)()));

    // destro temp resources
    vkDestroyBuffer(m_device, tempBuffer, nullptr);
    vkFreeMemory(m_device, tempMemory, nullptr);
}

MeshPool::Mesh MeshPool::GetHandle(std::string_view name)
{
    auto it = std::find(m_names.begin(), m_names.end(), name);
    if (it == m_names.end())
    {
        throw LettuceException(LettuceResult::NotFound);
    }
    uint32_t idx = (uint32_t)std::distance(m_names.begin(), it);
    return Mesh{ idx, weak_from_this() };
}