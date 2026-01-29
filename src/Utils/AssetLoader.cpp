// standard headers
#include <fstream>
#include <filesystem>
#include <memory>
#include <print>
#include <span>
#include <vector>

// external headers
#include <ktx.h>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Utils/api.hpp"
#include "Lettuce/Core/HelperStructs.hpp"
#include "Lettuce/Core/Allocators/LinearAllocator.hpp"

using namespace Lettuce::Utils;
using namespace Lettuce::Core::Allocators;

void AssetLoader::Create(Device& device, const AssetLoaderDesc& desc)
{
    m_device = &device;

    m_tempMem = std::make_unique<LinearAllocator>();
    m_resAlloc = std::make_unique<LinearAllocator>(); // temp, next replace with HeapAllocator

    LinearAllocatorDesc linDesc = {
        .bufferSize = desc.maxTempMemory,
        .imageSize = 16,
        .cpuVisible = true,
    };
    static_cast<LinearAllocator*>(m_tempMem.get())->Create(device, linDesc);
    
    linDesc.imageSize = desc.maxResourceMemory;
    linDesc.cpuVisible = false;
    static_cast<LinearAllocator*>(m_resAlloc.get())->Create(device, linDesc);

    CommandAllocatorDesc cmdDesc = {
        .queueType = QueueType::Copy,
    };
    m_cmds = device.CreateCommandAllocator(cmdDesc);
}

void AssetLoader::Destroy()
{
    m_device->Destroy(m_cmds);
    static_cast<LinearAllocator*>(m_resAlloc.get())->Destroy();
    static_cast<LinearAllocator*>(m_tempMem.get())->Destroy();
}

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
    return m_device->CreateShader(desc);
}

Texture AssetLoader::LoadKtx2Texture(std::string_view path, uint32_t levelCount, bool highQuality)
{
    // Lettuce Target is Desktop Platform & Modern Devices
    // So, needs to support
    // compressed:
    // Normal     HDR     
    // BC5        BC6H/BC7
    // byte-aligned RAW
    // non byte-aligned RAW

    if(!std::filesystem::exists(path))
    {
        DebugPrint("[ASSET LOADER]", "File not found: {}", path);
        return {};
    }

    ktxTexture2* kTexture;
    auto pathStr = std::string(path);
    auto res = ktxTexture2_CreateFromNamedFile(pathStr.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);

    // TODO: handle res
    switch (res)
    {
    case ktx_error_code_e::KTX_SUCCESS : break;
    default:
        DebugPrint("[ASSET LOADER]", "Load Error: {}", path);
        return {};
    }

    if (ktxTexture2_NeedsTranscoding(kTexture))
    {
        DebugPrint("[ASSET LOADER]", "image transcoding initializated");
        ktxTexture2_Destroy(kTexture);
        auto res = ktxTexture2_CreateFromNamedFile(pathStr.c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &kTexture);

        ktx_transcode_fmt_e tf = highQuality ? KTX_TTF_BC7_RGBA : KTX_TTF_BC1_OR_3;
        auto resTranscoding = ktxTexture2_TranscodeBasis(kTexture, tf, 0);
    }

    // get params & data
    uint32_t texWidth = kTexture->baseWidth;
    uint32_t texHeight = kTexture->baseHeight;
    uint32_t texMipLevels = kTexture->numLevels;

    uint64_t imageSize = kTexture->dataSize;
    uint8_t* texData = kTexture->pData;

    // staging buffer in gpu
    auto staging = m_tempMem->AllocateMemory(imageSize);
    memcpy(staging.cpuAddress, texData, imageSize);

    DebugPrint("[ASSET LOADER]", "image size: {} KB", imageSize / (1024));
    DebugPrint("[ASSET LOADER]", "image format: {}", kTexture->vkFormat);

    // may cause UB if it's not correct
    Format format = Core::FromVkFormat((VkFormat)(kTexture->vkFormat));

    uint64_t mainMipOffset;
    ktxTexture2_GetImageOffset(kTexture, 0, 0, 0, &mainMipOffset);

    // create long resource
    // TODO: impl kTexture->numLevels / mipLevels
    TextureDesc desc = {
        texWidth, texHeight, kTexture->baseDepth, format, 1, kTexture->numLayers, kTexture->isCubemap,
    };
    auto tex = m_resAlloc->AllocateTexture(desc);

    auto cmd = m_device->AllocateCommandBuffer(m_cmds);

    cmd.PrepareTexture(tex);
    // create copies for the first mipmap, next blit levels
    staging.offset += mainMipOffset;
    MemoryToTextureCopy copy = {
        .srcMemory = staging,
        .dstTexture = tex,
        .mipmapLevel = 0,
        .layerBaseLevel = 0,
        .layerCount = kTexture->numLayers,
    };
    cmd.MemoryCopy(copy);

    std::array<std::span<CommandBuffer>, 1> cmdArr = { std::span(&cmd, 1) };
    CommandBufferSubmitDesc cmdDesc = {
        .queueType = QueueType::Copy,
        .commandBuffers = std::span(cmdArr),
    };
    m_device->Submit(cmdDesc);

    // here the host waits; delete it for future async impls
    m_device->WaitFor(QueueType::Copy);

    static_cast<LinearAllocator*>(m_tempMem.get())->ResetMemory();
    ktxTexture2_Destroy(kTexture);

    DebugPrint("[ASSET LOADER]", "texture: {} loaded successfully", path);

    return tex;
}