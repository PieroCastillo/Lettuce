//
// Created by piero on 18/11/2024.
//
#include <iostream>
#include "Lettuce/X3D/commonX3D.hpp"
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
}

void Lettuce::X3D::Scene::loadMesh(fastgltf::Asset &asset, fastgltf::Mesh &meshData)
{
    Mesh mesh;

    // access to all primitives
    for (auto it = meshData.primitives.begin(); it != meshData.primitives.end(); ++it)
    {
        // get attributes
        auto *positionIt = it->findAttribute("POSITION");
        auto *normalIt = it->findAttribute("NORMAL");
        auto *tangentIt = it->findAttribute("TANGENT");

        // get accessors
        auto &positionAccessor = asset.accessors[positionIt->accessorIndex]; // Position Accessor
        auto &normalAccessor = asset.accessors[normalIt->accessorIndex];     // Normal Accessor
        auto &tangentAccessor = asset.accessors[tangentIt->accessorIndex];   // Tangent Accessor
        auto &indexAccessor = asset.accessors[it->indicesAccessor.value()];  // Indices Accessor

        // check if accesors have values
        if (!positionAccessor.bufferViewIndex.has_value() &&
            !normalAccessor.bufferViewIndex.has_value() &&
            !tangentAccessor.bufferViewIndex.has_value() &&
            !indexAccessor.bufferViewIndex.has_value())
            continue;

        // get values
        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, positionAccessor, [&](fastgltf::math::fvec3 pos, size_t idx)
                                                                  { mesh.positions.push_back({pos.x(), pos.y(), pos.z()}); });
        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, normalAccessor, [&](fastgltf::math::fvec3 nor, size_t idx)
                                                                  { mesh.normals.push_back({nor.x(), nor.y(), nor.z()}); });
        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(asset, tangentAccessor, [&](fastgltf::math::fvec4 tang, size_t idx)
                                                                  { mesh.tangents.push_back({tang.x(), tang.y(), tang.z(), tang.w()}); });
        fastgltf::iterateAccessorWithIndex<uint32_t>(asset, indexAccessor, [&](uint32_t index, size_t idx)
                                                     { mesh.indices.push_back(index); });
    }

    meshes.push_back(mesh);
}

void Lettuce::X3D::Scene::transferGeometry(void *bufferMemory, uint32_t bufferMemorySize)
{
    // create tmp pool    / geometry pool

    auto tmpPool = std::make_shared<Lettuce::Core::ResourcePool>();
    geometryBufferPool = std::make_shared<Lettuce::Core::ResourcePool>();
    transfer = std::make_shared<TransferManager>(device);

    // create tmp buffer / geometry buffer

    auto tmpBuffer = std::make_shared<BufferResource>(device, bufferMemorySize, VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT);
    geometryBuffer = std::make_shared<BufferResource>(device, bufferMemorySize, VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_2_TRANSFER_DST_BIT);

    // add buffers to their respectives pools
    tmpPool->AddResource(tmpBuffer);
    geometryBufferPool->AddResource(geometryBuffer);

    // bind resources
    tmpPool->Bind(device, VK_MEMORY_PROPERTY_HOST_CACHED_BIT);             // create pool in host memory
    geometryBufferPool->Bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // create pool in device memory

    // copy data

    tmpPool->Map(0, bufferMemorySize);
    tmpPool->SetData((void *)bufferMemory, 0, bufferMemorySize);
    tmpPool->UnMap();
    free((void *)bufferMemory); // WARNING : here we are deleting memory

    // transfer
    transfer->Prepare();
    transfer->AddTransference(tmpBuffer, geometryBuffer, TransferType::HostToDevice);
    transfer->TransferAll();

    tmpBuffer->Release();
    tmpPool->Release();
}

void Lettuce::X3D::Scene::LoadFromFile(const std::shared_ptr<Lettuce::Core::Device> &device, std::filesystem::path path)
{
    this->device = device;
    // loads gltf file
    auto data = fastgltf::GltfDataBuffer::FromPath(path);
    if (data.error() != fastgltf::Error::None)
    {
        return;
    }
    std::cout << "data obtained successfully" << std::endl;
    fastgltf::Parser parser;
    auto asset = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::GenerateMeshIndices);
    if (auto error = asset.error(); error != fastgltf::Error::None)
    {
        return;
    }
    std::cout << "asset loaded successfully" << std::endl;

    // load all meshes
    // alloc all in one big pool
    // FREE pointers
    {
        std::vector<std::tuple<char *, uint32_t>> datas; // ptr, size
        uint32_t indexCount = 0;
        uint32_t vertexBufferSize = 0;
        int i = 0;
        for (auto &mesh : asset->meshes)
        {
            loadMesh(asset.get(), mesh);

            // create memory blocks

            uint32_t size = (sizeof(glm::vec3) * (meshes[i].positions.size() + meshes[i].normals.size())) + (sizeof(glm::vec4) * meshes[i].tangents.size());

            char *alloc = (char *)malloc(size); // WARNING : it's creating a raw pointer
            char *allocIdx = alloc;
            memcpy((void *)allocIdx, (void *)meshes[i].positions.data(), sizeof(glm::vec3) * meshes[i].positions.size());
            allocIdx += sizeof(glm::vec3) * meshes[i].positions.size();

            memcpy((void *)allocIdx, (void *)meshes[i].normals.data(), sizeof(glm::vec3) * meshes[i].normals.size());
            allocIdx += sizeof(glm::vec3) * meshes[i].normals.size();

            memcpy((void *)allocIdx, (void *)meshes[i].tangents.data(), sizeof(glm::vec3) * meshes[i].tangents.size());

            datas.push_back({alloc, size});

            // set vertex offsets

            meshInfos[i].vertexBufferSize = size;
            meshInfos[i].vertexBufferOffset = vertexBufferSize;
            meshInfos[i].vertexCount = (uint32_t)meshes[i].positions.size();

            vertexBufferSize += size;
            // add index count
            indexCount += meshes[i].indices.size();

            i++;
        }
        std::cout << "meshes geometries loaded successfully" << std::endl;

        // set index offsets
        uint32_t indexBufferOffset = vertexBufferSize;
        int j = 0;
        for (auto &mesh : asset->meshes)
        {
            meshInfos[j].indexBufferSize = sizeof(uint32_t) * meshes[j].indices.size();
            meshInfos[j].indexBufferOffset = indexBufferOffset;
            indexBufferOffset += sizeof(uint32_t) * meshes[j].indices.size();

            j++;
        }

        std::cout << "meshes loaded successfully" << std::endl;

        // release memory used by mesh vector
        meshes.clear();
        meshes.shrink_to_fit();

        uint32_t indexBufferSize = sizeof(uint32_t) * indexCount;
        uint32_t bufferMemorySize = vertexBufferSize + indexBufferSize;

        char *bufferMemory = (char *)malloc(bufferMemorySize); // WARNING : it's creating a raw pointer
        char *bufferMemoryIdx = bufferMemory;

        /*
        The structure of the geometry buffer is:
        |  vertices block  |   indices block    |
        | P | N | T | .... | i | ...            |
        */

        for (auto &[ptr, size] : datas)
        {
            memcpy((void *)bufferMemoryIdx, (void *)ptr, size);
            bufferMemoryIdx += size;
            free((void *)ptr); // WARNING : here we are deleting memory
        }
        // here the only non-released pointer must be bufferMemory

        for (auto &mesh : meshes)
        {
            memcpy((void *)bufferMemoryIdx, (void *)mesh.indices.data(), mesh.indices.size());
            bufferMemoryIdx += mesh.indices.size();
        }

        transferGeometry((void *)bufferMemory, bufferMemorySize);
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

    for (auto &node : asset->nodes)
    {
        std::vector<int> children;
        children.resize(node.children.size());
        for (int i = 0; i < node.children.size(); i++)
        {
            children[i] = node.children[i];
        }

        glm::mat4 transform = glm::identity<glm::mat4>();

        if (std::holds_alternative<fastgltf::math::fmat4x4>(node.transform))
        {
            auto m = std::get<1>(node.transform);
            transform = glm::mat4(m[0][0], m[0][1], m[0][2], m[0][3],
                                  m[1][0], m[1][1], m[1][2], m[1][3],
                                  m[2][0], m[2][1], m[2][2], m[2][3],
                                  m[3][0], m[3][1], m[3][2], m[3][3]);
        }

        nodes.emplace_back(children, (int)node.meshIndex.value(), std::move(transform));
    }

    auto defScene = asset->scenes[asset->defaultScene.value()];
    for (auto &idx : defScene.nodeIndices)
    {
        nodesIndices.push_back(idx);
    }
}

void Lettuce::X3D::Scene::DrawIndexed(VkCommandBuffer cmd)
{
    std::vector<int> instancedNodeIndices(nodesIndices);

    for (auto &idx : instancedNodeIndices)
    {
        // get mesh
        auto mesh = meshInfos[nodes[idx].meshIndex];
        // draw indexed
        vkCmdBindVertexBuffers2(cmd, 0, 1, geometryBuffer->GetHandlePtr(), (VkDeviceSize *)&(mesh.vertexBufferOffset), (VkDeviceSize *)&(mesh.vertexBufferSize), nullptr);
        vkCmdBindIndexBuffer(cmd, geometryBuffer->GetHandle(), (VkDeviceSize)mesh.indexBufferOffset, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, mesh.vertexCount, 1, 0, 0, 0);
        // copy node indices
        instancedNodeIndices.insert(instancedNodeIndices.end(), nodes[idx].children.begin(), nodes[idx].children.end());
    }
}

void Lettuce::X3D::Scene::Release()
{
    geometryBufferPool->Release();
    geometryBuffer->Release();
}