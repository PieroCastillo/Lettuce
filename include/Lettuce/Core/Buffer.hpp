//
// Created by piero on 26/06/2024
//
#pragma once
#include <iostream>
#include <vector>
#include <volk.h>
#include "VmaUsage.hpp"
#include "Device.hpp"
#include "Utils.hpp"

namespace Lettuce::Core
{
    enum class MemoryProperty : uint32_t
    {
        DeviceLocal = 0x00000001,     // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        HostVisible = 0x00000002,     // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        HostCoherent = 0x00000004,    // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        HostCached = 0x00000008,      // VK_MEMORY_PROPERTY_HOST_CACHED_BIT
        LazilyAllocated = 0x00000010, // VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
        Protected = 0x00000020,       // VK_MEMORY_PROPERTY_PROTECTED_BIT
    };
    MAKE_ENUM_FLAGS(MemoryProperty)

    enum class BufferUsage : uint32_t
    {
        TransferSrc = 0x00000001,
        TransferDst = 0x00000002,
        UniformTexelBuffer = 0x00000004,
        StorageTexelBuffer = 0x00000008,
        UniformBuffer = 0x00000010,
        StorageBuffer = 0x00000020,
        IndexBuffer = 0x00000040,
        VertexBuffer = 0x00000080,
        IndirectBuffer = 0x00000100,
        ShaderDeviceAdress = 0x00020000,
        VideoDecodeSrc = 0x00002000,
        VideoDecodeDst = 0x00004000,
        TransformFeedbackBuffer = 0x00000800,
        TransformFeedbackCounterBuffer = 0x00001000,
        ConditionalRendering = 0x00000200,
        AccelerationStructureBuildInputReadOnly = 0x00080000,
        AccelerationStructureStorage = 0x00100000,
        ShaderBindingTable = 0x00000400,
        VideoEncodeDst = 0x00008000,
        VideoEncodeSrc = 0x00010000,
        SamplerDescriptorBuffer = 0x00200000,
        ResourceDescriptorBuffer = 0x00400000,
        PushDescriptorsDescriptorBuffer = 0x04000000,
        MicromapBuildInputReadOnly = 0x00800000,
        MicromapStorage = 0x01000000,
    };
    MAKE_ENUM_FLAGS(BufferUsage)

    class Buffer
    {
    public:
        Device _device;
        VkBuffer _buffer;
        VmaAllocation _allocation;
        VkCommandPool _pool;
        uint32_t _size;
        BufferUsage _usage;

        /// @brief Creates a Buffer, this is a contiguous block of memory thats holds some data
        /// @param device the logical device
        /// @param size size of the buffer, in bytes
        /// @param usage usage of the buffer
        /// @param allocationFlags
        /// @param memoryUsage
        void Create(Device &device, uint32_t size, BufferUsage usage,
                    VmaAllocationCreateFlags allocationFlags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                    VmaMemoryUsage memoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO);

        /// @brief Maps data from host memory to buffer
        /// @param src pinter to data to be mapped
        void SetData(void *src);

        /// @brief Copy data from host visible Buffer to device prefered Buffer, 
        /// the usage of this function is for staging buffers creation only, another
        /// usage may have unexpected behaviors.
        /// @param buffer the dst buffer
        void CopyTo(Buffer buffer);

        template <typename T>
        static Buffer CreateVertexBuffer(Device &device, std::vector<T> vertices);

        template <typename T, typename = std::enable_if_t<
                                  std::is_integral<T>::value && std::is_unsigned<T>::value>>
        static Buffer CreateIndexBuffer(Device &device, std::vector<T> indices);

        static Buffer CreateBufferWithStaging(Device &device, BufferUsage bufferDstUsage, uint32_t size, void *data);

        template <typename T>
        static Buffer CreateUniformBuffer(Device &device, T **data);

        void Destroy();
    };
}