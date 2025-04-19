//
// Created by piero on 13/11/2024.
//
#pragma once
#include <vector>
#include <memory>

#include "commonX3D.hpp"
#include "Mesh.hpp"

#include "Lettuce/Core/ResourcePool.hpp"
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/TransferManager.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/IReleasable.hpp"

namespace Lettuce::X3D
{
    struct Node
    {
        std::vector<int> children;
        int meshIndex;
        glm::mat4 transform;
        
        Node(const std::vector<int>& children,
             int meshIndex,
             glm::mat4 transform) : children(children),
                                    meshIndex(meshIndex),
                                    transform(transform)
        {
        }
    };

    class Scene : public Lettuce::Core::IReleasable
    {
    private:
        // load results
        bool result;
        std::string err;
        std::string warn;
        // main data
        std::shared_ptr<Lettuce::Core::ResourcePool> geometryBufferPool;
        std::shared_ptr<Lettuce::Core::BufferResource> geometryBuffer;
        std::shared_ptr<Lettuce::Core::TransferManager> transfer;
        std::shared_ptr<Lettuce::Core::Device> device;

        std::vector<Mesh> meshes;
        std::vector<MeshInfo> meshInfos;
        std::vector<Node> nodes;
        std::vector<int> nodesIndices;

        void check();
        void setup();
        void loadNode();

        void loadMesh(fastgltf::Asset &asset, fastgltf::Mesh &mesh);
        void transferGeometry(void* bufferMemory, uint32_t bufferMemorySize);

    public:
        Scene() {}
        void LoadFromFile(const std::shared_ptr<Lettuce::Core::Device> &device, std::filesystem::path path);

        void DrawIndexed(VkCommandBuffer cmd);

        void Release();
    };
}