//
// Created by piero on 19/02/2025.
//
#pragma once
#include "Lettuce/Core/Primitive.hpp"
#include <memory>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/BufferResource.hpp"

using namespace Lettuce::Core;

namespace Lettuce::X3D
{
    struct PrimitiveInfo
    {
        BufferBlock vertBlock;
        BufferBlock indexBlock;
    };

    class PrimitiveManager
    {
    public:
    PrimitiveManager(const std::shared_ptr<Device> &device);
    std::shared_ptr<BufferResource> TransferToDeviceMemory(std::vector<Primitive> primitives);
    void Release();
    };
}