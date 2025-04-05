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

void *getPointerFromRawBuffer(fastgltf::DataSource dataSrc)
{
    void *ptr;

    if (std::holds_alternative<fastgltf::sources::URI>(dataSrc))
    {
        auto value = std::get<fastgltf::sources::URI>(dataSrc);
    }
    else if (std::holds_alternative<fastgltf::sources::Array>(dataSrc))
    {
        auto value = std::get<fastgltf::sources::Array>(dataSrc);
        ptr = (void *)value.bytes.data();
    }
    else if (std::holds_alternative<fastgltf::sources::Vector>(dataSrc))
    {
        auto value = std::get<fastgltf::sources::Vector>(dataSrc);
        ptr = (void *)value.bytes.data();
    }
    else if (std::holds_alternative<fastgltf::sources::ByteView>(dataSrc))
    {
        auto value = std::get<fastgltf::sources::ByteView>(dataSrc);
        ptr = (void *)value.bytes.data();
    }
    else if (std::holds_alternative<fastgltf::sources::BufferView>(dataSrc))
    {
        auto value = std::get<fastgltf::sources::BufferView>(dataSrc);
    }
    else if (std::holds_alternative<fastgltf::sources::CustomBuffer>(dataSrc))
    {
        auto value = std::get<fastgltf::sources::CustomBuffer>(dataSrc);
    }
    else if (std::holds_alternative<fastgltf::sources::Fallback>(dataSrc))
    {
        auto value = std::get<fastgltf::sources::Fallback>(dataSrc);
    }
    return ptr;
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
    /*
    - gltf buffer: contains info of length, name and the data itself
    - gltf bufferview: info of index, length, offset, stride, name and target
    */
    // here we load the buffers of the scene
    // and create temporal buffers (located in host memory)
    _buffers.reserve(asset->buffers.size());
    ResourcePool tempBuffersPool;
    std::vector<std::shared_ptr<BufferResource>> tempBuffers; // create temporal buffer, flags: src
    tempBuffers.reserve(_buffers.size());
    for (auto &buffer : asset->buffers)
    {
        auto Lbuffer = std::make_shared<Lettuce::Core::BufferResource>(_device, buffer.byteLength,
                                                                       VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                                           VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                                           VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        _buffers.push_back(Lbuffer);
        _buffersPool->AddResource(Lbuffer);

        auto temp = std::make_shared<BufferResource>(_device, buffer.byteLength, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        tempBuffers.push_back(temp);
        tempBuffersPool.AddResource(temp);
    }
    // we use device local memory because we don't need to read this memory from host
    _buffersPool->Bind(device, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    tempBuffersPool.Bind(device, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // we created the buffers and we've allocated the memory
    // now, we need to tranfers the resources from the temporal to the device-memory foreach buffer
    VkCommandPool cmdPool;
    VkCommandBuffer cmd;
    // vkBeginCommandBuffer(cmd, )
    uint32_t offset = 0;
    void *bufferData;
    
    // map all memory to one lettuce buffer
    for (auto &buffer : asset->buffers)
    {
        // copy raw data to temp buffer
        vkMapMemory(_device->_device, tempBuffersPool._memory, offset, buffer.byteLength, 0, &bufferData);
        memcpy(bufferData, getPointerFromRawBuffer(buffer.data), buffer.byteLength);
        // memccpy(data, buffer.data)
        vkUnmapMemory(_device->_device, tempBuffersPool._memory);
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

    for (auto &mesh : asset->meshes)
    {

    }

    // create virtual buffer (blocks) for recornized types
    // note: accessor.componenType : uint, int, ...
    //       accessor.accessorType : vec2, scalar, ...
    for (auto &accessor : asset->accessors)
    {
        
    }

}

void Lettuce::X3D::Scene::Release()
{
    for (auto &buffer : _buffers)
    {
        buffer->Release(); // delete all references
    }
    _buffersPool->Release();
}