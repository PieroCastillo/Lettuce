/*
Created by @PieroCastillo on 2025-10-24
*/
#ifndef LETTUCE_CORE_MESH_HPP
#define LETTUCE_CORE_MESH_HPP
// standard headers
#include <vector>
#include <memory>

// project headers
#include "common.hpp"

// external headers 
#define FASTGLTF_COMPILE_AS_CPP20
#include "fastgltf/types.hpp"

namespace Lettuce::Core
{
    // set of primitives of a Mesh
    struct PrimitiveDatas
    {
        std::vector<void*> primitiveDataPtr;
        std::vector<uint32_t> primitiveMemorySize;
        std::vector<uint32_t> vertexCount;
        std::vector<uint32_t> indexCount;
        std::vector<uint32_t> posOffset;
        std::vector<uint32_t> norOffset;
        std::vector<uint32_t> tanOffset;
        std::vector<uint32_t> idxOffset;
    };

    struct MeshPoolCreateInfo
    {

    };

    /*
    |-----------------Allocation--------------------|
    |------------------Buffer-----------------------|
    |                   Mesh 1                | ... |
    |               P1             |-P2 -| ...| ... |
    | Pos | Norm | Tex | ... | Idx |
    */
    class MeshPool : std::enable_shared_from_this<MeshPool>
    {
    private:
        std::vector<PrimitiveDatas> m_primitiveDatas;
        std::vector<std::string> m_names;

        void setupMeshBufferMemory(VkDeviceMemory* memory, VkBuffer* buffer, bool isStaging);
        uint32_t m_size;

        VkCommandPool m_cmdPool;
        VkCommandBuffer m_cmdBuffer;
        VkQueue m_transferQueue;
        uint32_t m_transferQueueFamilyIndex;
        VkFence m_fence;
    public:
        struct Mesh
        {
            uint32_t m_index;
            std::weak_ptr<MeshPool> m_meshPool;

            bool valid() const {
                auto p = m_meshPool.lock();
                return p && m_index < p->m_primitiveDatas.size();
            }
        };

        VkDevice m_device;
        VkPhysicalDevice m_gpu;
        VkDeviceMemory m_memory;
        VkBuffer m_buffer;

        void Create(const IDevice& device, const MeshPoolCreateInfo& createInfo);
        void Release();

        void Load(const fastgltf::Asset& asset);

        Mesh GetHandle(std::string_view name);
    };
};
#endif // LETTUCE_CORE_MESH_HPP