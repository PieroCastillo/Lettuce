/*
Created by @PieroCastillo on 2025-10-24
*/
#ifndef LETTUCE_CORE_MESH_HPP
#define LETTUCE_CORE_MESH_HPP
// standard headers

// project headers
#include "common.hpp"

// external headers 
#define FASTGLTF_COMPILE_AS_CPP20
#include "fastgltf/types.hpp"

namespace Lettuce::Core
{
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
    class MeshPool
    {
    private:
        struct PrimitiveDatas
        {
            std::vector<uint32_t> primitiveMemorySize;
            std::vector<int> VertexCount;
            std::vector<int> indexCount;
            std::vector<uint32_t> posMemSize;
            std::vector<uint32_t> norMemSize;
            std::vector<uint32_t> tanMemSize;
            std::vector<uint32_t> idxMemSize;
            std::vector<uint32_t> posOffset;
            std::vector<uint32_t> norOffset;
            std::vector<uint32_t> tanOffset;
            std::vector<uint32_t> idxOffset;
        };

        void setupMeshBufferMemory(VkDeviceMemory* memory, VkBuffer* buffer, bool isStaging);
        uint32_t m_size;
    public:
        VkDevice m_device;
        VkPhysicalDevice m_gpu;
        VkDeviceMemory m_memory;
        VkBuffer m_buffer;

        void Create(const IDevice& device, const MeshPoolCreateInfo& createInfo);
        void Release();

        void Load(const fastgltf::Asset& asset);

        // TODO: get mesh handle by name
    };
};
#endif // LETTUCE_CORE_MESH_HPP