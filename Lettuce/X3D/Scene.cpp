//
// Created by piero on 18/11/2024.
//
#include <iostream>
#include "Lettuce/X3D/Scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

using namespace Lettuce::Core;

void Lettuce::X3D::Scene::check()
{
    if (!warn.empty())
    {
        std::cout << "Warn: " << warn << std::endl;
    }

    if (!err.empty())
    {
        std::cout << "Error: " << err << std::endl;
    }

    if (!result)
    {
        std::cout << "Failed to parse glTF" << std::endl;
        return;
    }
}

void Lettuce::X3D::Scene::setup()
{
    check();
    // buffers->resize(model.buffers.size()); //reserve space to access to this memory more faster
    // int bufferIndex = 0;
    // for(auto bufferGltf : model.buffers)
    // {
    //     //bufferGltf.
    //     // buffers[bufferIndex].
    //     //bufferIndex++;
    // }
    // model.meshes[0].primitives[0].attributes
}

void Lettuce::X3D::Scene::loadMesh(fastgltf::Asset &asset, fastgltf::Mesh &mesh)
{
    Mesh mesh;

    // access to all primitives
    for (auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it)
    {
        // Position Accessor
        auto *positionIt = it->findAttribute("POSITION");
        auto &positionAccessor = asset.accessors[positionIt->accessorIndex];

        if (!positionAccessor.bufferViewIndex.has_value())
            continue;

        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, positionAccessor, [&](fastgltf::math::fvec3 pos, size_t idx)
                                                                  { mesh.positions.push_back({pos.x, pos.y, pos.z}); });

        // Normal Accessor
        auto *normalIt = it->findAttribute("NORMAL");
        auto &normalAccessor = asset.accessors[normalIt->accessorIndex];

        if (!normalAccessor.bufferViewIndex.has_value())
            continue;

        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, normalAccessor, [&](fastgltf::math::fvec3 nor, size_t idx)
                                                                  { mesh.normals.push_back({nor.x, nor.y, nor.z}); });

        // Tangent Accessor
        auto *tangentIt = it->findAttribute("TANGENT");
        auto &tangentAccessor = asset.accessors[tangentIt->accessorIndex];

        if (!tangentAccessor.bufferViewIndex.has_value())
            continue;

        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(asset, tangentAccessor, [&](fastgltf::math::fvec4 tang, size_t idx)
                                                                  { mesh.tangents.push_back({tang.x, tang.y, tang.z, tang.w}); });

        // Indices Accessor

        auto &indexAccessor = asset.accessors[it->indicesAccessor.value()];
        if (!indexAccessor.bufferViewIndex.has_value())
            continue;

        fastgltf::iterateAccessorWithIndex<std::uint32_t *>(asset, indexAccessor, [&](std::uint32_t *index, size_t idx)
                                                            { mesh.indices.push_back(*index); });
    }

    meshes.push_back(mesh);
}

void Lettuce::X3D::Scene::LoadFromFile(const std::shared_ptr<Lettuce::Core::Device> &device, std::filesystem::path path)
{
    _device = device;
    // loads gltf file
    auto data = fastgltf::GltfDataBuffer::FromPath(path);
    if (data.error() != fastgltf::Error::None)
    {
        return;
    }
    fastgltf::Parser parser;
    auto asset = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::None);
    if (auto error = asset.error(); error != fastgltf::Error::None)
    {
        return;
    }

    // load all meshes
    // retrieve info of vertices
    // alloc all in one big pool
    // FREE pointers
    {

        for (auto &mesh : asset->meshes)
        {
            loadMesh(asset.get(), mesh);
        }

        // create tmp pool    / geometry pool

        auto tmpPool = std::make_shared<Lettuce::Core::ResourcePool>();
        geometryBufferPool = std::make_shared<Lettuce::Core::ResourcePool>();
        auto transfer = std::make_shared<TransferManager>(device);

        // create tmp buffer / geometry buffer

        auto tmpBuffer = std::make_shared<BufferResource>(device, geometryBufferSize, VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT);
        geometryBuffer = std::make_shared<BufferResource>(device, geometryBufferSize, VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_2_TRANSFER_DST_BIT);

        // add buffers to their respectives pools
        tmpPool->AddResource(tmpBuffer);
        geometryBufferPool->AddResource(geometryBuffer);

        // bind resources
        tmpPool->Bind(device, VK_MEMORY_PROPERTY_HOST_CACHED_BIT);             // create pool in host memory
        geometryBufferPool->Bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // create pool in device memory

        // copy data

        

        // transfer

    }

    for (auto &img : asset->images)
    {
    }

    for (auto &sampler : asset->samplers)
    {
    }

    for (auto &texture : asset->textures)
    {
    }
}

void Lettuce::X3D::Scene::Release()
{
    geometryBufferPool->Release();
    geometryBuffer->Release();
}