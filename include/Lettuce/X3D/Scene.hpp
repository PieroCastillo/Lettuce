//
// Created by piero on 13/11/2024.
//
#pragma once
#include <vector>
#include <memory>
#include "commonX3D.hpp"
#include "Lettuce/Core/ResourcePool.hpp"
#include "Lettuce/Core/BufferResource.hpp"
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
        std::shared_ptr<Lettuce::Core::ResourcePool> _buffersPool;
        std::shared_ptr<Lettuce::Core::Device> _device;
        std::vector<std::shared_ptr<Lettuce::Core::BufferResource>> _buffers;  
        void check();
        void setup();

    public:
        Scene() {}
        void LoadFromFile(const std::shared_ptr<Lettuce::Core::Device>& device, std::filesystem::path path);

        void Release();
    };
}