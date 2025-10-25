// project headers
#include "Lettuce/Core/Mesh.hpp"

// external headers
#include <fastgltf/tools.hpp>

using namespace Lettuce::Core;

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
        memoryAI.memoryTypeIndex = findMemoryTypeIndex(m_device, m_gpu, memReqs.memoryTypeBits, MemoryAccess::FastCPUWriteGPURead);
    }

    handleResult(vkAllocateMemory(m_device, &memoryAI, nullptr, memoryPtr));

    handleResult(vkBindBufferMemory(m_device, *bufferPtr, *memoryPtr, 0));
}

void MeshPool::Create(const IDevice& device, const MeshPoolCreateInfo& createInfo)
{
    m_device = device.m_device;
    m_gpu = device.m_physicalDevice;
}

void MeshPool::Release()
{
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_memory, nullptr);
}

void MeshPool::Load(const fastgltf::Asset& asset)
{
    /*
    equivalent: 
    struct PrimitiveData {
        uint32_t primitiveMemorySize;
        int VertexCount;
        int indexCount;
        uint32_t posMemSize;
        uint32_t norMemSize;
        uint32_t tanMemSize;
        uint32_t idxMemSize;
        uint32_t posOffset;
        uint32_t norOffset;
        uint32_t tanOffset;
        uint32_t idxOffset;
    }
    */
    std::vector<std::vector<uint32_t>> primitiveOffsets;
    std::vector<void*> primitivesMemories;
    std::vector<int> vertexCount;
    std::vector<uint32_t> primitiveSizes;
    std::vector<uint32_t> posOffsets;
    std::vector<uint32_t> norOffsets;
    std::vector<uint32_t> tanOffsets;
    std::vector<uint32_t> idxOffsets;

    for (const auto& mesh : asset.meshes)
    {
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

            uint32_t posASize = posA.count*3*sizeof(float);
            uint32_t norASize = norA.count*3*sizeof(float);
            uint32_t tanASize = tanA.count*4*sizeof(float);
            uint32_t idxASize = idxA.count*sizeof(uint32_t);

            uint32_t primSize = posASize + norASize + tanASize + idxASize;
            void* primMem = malloc(primSize);
            
            int* tempPtr = (int*)primMem;
            fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, posA, tempPtr);
            tempPtr+=posASize;
            fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, norA, tempPtr);
            tempPtr+=norASize;
            fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, tanA, tempPtr);
            tempPtr+=tanASize;
            fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, idxA, tempPtr);
        }
    }
}