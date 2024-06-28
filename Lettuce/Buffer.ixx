//
// Created by piero on 26/06/2024
//
module;
#include <iostream>
#include <vector>
#define VOLK_IMPLEMENTATION
#include <volk.h>

export module Lettuce:Buffer;

import :Device;
import :Utils;

export namespace Lettuce::Core
{
    enum class BufferUsage
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

    // template <typename T>
    class Buffer
    {
        Device _device;
        VkBuffer _buffer;

        void Create(Device &device)
        {
            _device = device;

            // VkBufferCreateInfo bufferCI = {
            //     .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            //     //.usage = VkBufferUsageFlagBits::
            //     .size = sizeof(T),
            // };

            // checkResult(vkCreateBuffer(_device._device, &bufferCI, nullptr, &_buffer), "buffer created sucessfully");
        }
    };
}