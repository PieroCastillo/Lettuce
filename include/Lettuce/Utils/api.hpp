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
    struct AssetLoaderDesc
    {
        uint64_t maxTempMemory;
        uint64_t maxResourceMemory;
    };

    struct AssetLoader
    {
    private:
        Device m_device;
        CommandAllocator m_cmds;
        std::unique_ptr<IGPUMemoryResource> m_tempMem;
        std::unique_ptr<IGPUMemoryResource> m_resAlloc;
    public:
        void Create(Device, const AssetLoaderDesc&);
        void Destroy();
        ShaderBinary LoadSpirv(std::string_view path);
        Texture LoadKtx2Texture(std::string_view path, uint32_t levels = 1, bool highQuality = false);
        // LoadGltfModel(std::string_view);
    };
};

#endif // LETTUCE_UTILS_API_HPP