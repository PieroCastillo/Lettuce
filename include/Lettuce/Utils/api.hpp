/*
Created by @PieroCastillo on 2025-01-5
*/
#ifndef LETTUCE_UTILS_API_HPP
#define LETTUCE_UTILS_API_HPP

#include <array>
#include <cstdint>
#include <memory>
#include <string_view>

#include "../Core/api.hpp"
#include "../Core/Allocators/IGPUMemoryResource.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Core::Allocators;

namespace Lettuce::Utils
{
    struct AssetLoader
    {
    private:
        Device m_device;
        CommandAllocator m_cmds;
        std::unique_ptr<IGPUMemoryResource> m_tempMem;
        std::unique_ptr<IGPUMemoryResource> m_resAlloc;
    public:
        ShaderBinary LoadSpirv(std::string_view path);
        Texture LoadKtx2Texture(std::string_view path);
        // LoadGltfModel()
    };
};

#endif // LETTUCE_UTILS_API_HPP