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

namespace Lettuce::X3D
{
    class Scene
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
        std::shared_ptr<Lettuce::Core::Device> _device;

        std::vector<Mesh> meshes;

        void check();
        void setup();

        void loadMesh(fastgltf::Asset &asset, fastgltf::Mesh &mesh);

    public:
        Scene() {}
        void LoadFromFile(const std::shared_ptr<Lettuce::Core::Device> &device, std::filesystem::path path);

        void Release();
    };
}