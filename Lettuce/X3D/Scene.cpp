//
// Created by piero on 18/11/2024.
//
#include <iostream>
#include "Lettuce/X3D/Scene.hpp"

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
    std::vector<VerticesInfo> infos;
    std::vector<VerticesData> datas;
    // access to all primitives
    for (auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it)
    {
        std::vector<VkFormat> formats = {VK_FORMAT_R32G32B32_SFLOAT};

        auto *positionIt = it->findAttribute("POSITION");
        auto &positionAccessor = asset.accessors[positionIt->accessorIndex];

        if (!positionAccessor.bufferViewIndex.has_value())
            continue;

        // here continue adding accessors

        int size = 4 * 3 * positionAccessor.count; // vec3 float
        int stride = 4 * 3;                        // fvec3
        int elementSize = 4 * 3;                   // fvec3
        // WARNING: here you are creating unsafe pointers,  you SHOULD FREE these after
        void *bufferPtr = malloc(size);
        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, positionAccessor, [&](fastgltf::math::fvec3 pos, size_t idx)
                                                                  {
            char* bufferIdx = (char*)bufferPtr + idx;
            memcpy((void*)bufferIdx, (void*)pos.data(), elementSize); });

        infos.push_back({(uint32_t)positionAccessor.count, formats});
        datas.push_back({(uint32_t)size, bufferPtr});

        geometryBufferSize += size;
    }
    sceneVerticesInfos.push_back(infos);
    sceneVerticesDatas.push_back(datas);
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

        // create tmp buffer / geometry buffer



        // copy data
        // transfer

        // release pointers
        for (auto &datas : sceneVerticesDatas)
        {
            for (auto &data : datas)
            {
                free(data.data);
            }
        }
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