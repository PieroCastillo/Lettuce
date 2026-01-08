// standard headers
#include <fstream>
#include <filesystem>
#include <memory>
#include <span>
#include <vector>

// external headers
#include <ktx.h>

// project headers
#include "Lettuce/Utils/api.hpp"
#include "Lettuce/Core/HelperStructs.hpp"

using namespace Lettuce::Utils;

ShaderBinary AssetLoader::LoadSpirv(std::string_view path)
{
    auto shadersFile = std::ifstream(std::string(path), std::ios::ate | std::ios::binary);
    if (!shadersFile) throw std::runtime_error(std::string(path) + " does not exist");

    auto fileSize = (uint32_t)shadersFile.tellg();
    std::vector<uint32_t> shadersBuffer;
    shadersBuffer.resize(fileSize / sizeof(uint32_t));

    shadersFile.seekg(0);
    shadersFile.read((char*)shadersBuffer.data(), fileSize);

    ShaderBinaryDesc desc = {
        .bytecode = shadersBuffer,
    };
    return m_device.CreateShader(desc);
}

Texture AssetLoader::LoadKtx2Texture(std::string_view path)
{
    // Lettuce Target is Desktop Platform & Modern Devices
    // So, needs to support
    // compressed:
    // Normal     HDR     
    // BC5  BC6H/BC7
    // byte-aligned RAW
    // non byte-aligned RAW

    ktxTexture2* kTexture;
    auto pathStr = std::string(path);
    auto res = ktxTexture2_CreateFromNamedFile(pathStr.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);

    // TODO: handle res

    // get params & data
    uint32_t texWidth = kTexture->baseWidth;
    uint32_t texHeight = kTexture->baseHeight;
    uint32_t texMipLevels = kTexture->numLevels;

    if(ktxTexture2_NeedsTranscoding(kTexture))
    {
        // TODO: impl basis universal
    }

    uint64_t imageSize = kTexture->dataSize;
    uint8_t* texData = kTexture->pData;

    // staging buffer in gpu
    auto staging = m_tempMem->AllocateMemory(imageSize);
    memcpy(staging.cpuAddress, texData, imageSize);

    // may cause UB if it's not correct
    Format format = Core::FromVkFormat((VkFormat)(kTexture->vkFormat));

    // create long resource
    TextureDesc desc = {
        texWidth, texHeight, kTexture->baseDepth, format, kTexture->numLevels, kTexture->numLayers, kTexture->isCubemap,
    };
    auto tex = m_resAlloc->AllocateTexture(desc);
    auto cmd = m_device.AllocateCommandBuffer(m_cmds);

    // create copies for all mipmaps
    MemoryToTextureCopy copy = {
        .dstTexture = tex,
        .layerBaseLevel = 0,
        .layerCount = kTexture->numLayers,
    };
    uint64_t offset;
    for (uint32_t lev = 0; lev < kTexture->numLevels; ++lev)
    {
        auto copMem = staging;
        ktxTexture2_GetImageOffset(kTexture, lev, 0, 0, &offset);
        copMem.offset += offset;
        copy.srcMemory = copMem;
        copy.mipmapLevel = lev;
        cmd.MemoryCopy(copy);
    }

    std::array<std::span<CommandBuffer>, 1> cmdArr = { std::span(&cmd, 1) };
    CommandBufferSubmitDesc cmdDesc = {
        .queueType = QueueType::Copy,
        .commandBuffers = std::span(cmdArr),
    };
    m_device.Submit(cmdDesc);

    // here the host waits; delete it for future async impls
    m_device.WaitFor(QueueType::Copy);

    return tex;
}