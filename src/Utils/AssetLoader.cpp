// standard headers
#include <fstream>
#include <filesystem>
#include <memory>
#include <print>
#include <span>
#include <system_error>
#include <vector>

// external headers
#include "glfw/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <ktx.h>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <meshoptimizer.h>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Utils/AssetLoader.hpp"
#include "Lettuce/Core/HelperStructs.hpp"

using namespace Lettuce::Utils;

auto AssetLoader::LoadSpirv(Device* pDevice, std::string_view path) -> ShaderBinary
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
    return pDevice->CreateShader(desc);
}

auto AssetLoader::LoadKtx2Texture(Device* pDevice, CommandAllocator commandAllocator, std::string_view path, uint32_t levelCount, bool highQuality) -> TextureView
{
    // Lettuce Target is Desktop Platform & Modern Devices
    // So, needs to support
    // compressed:
    // Normal     HDR     
    // BC5        BC6H/BC7
    // byte-aligned RAW
    // non byte-aligned RAW

    if (!std::filesystem::exists(path))
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
    case ktx_error_code_e::KTX_SUCCESS: break;
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
    MemoryViewDesc stagingInfo = {
        .size = imageSize,
        .cpuVisible = true,
    };
    auto stagingBuff = pDevice->CreateMemoryView(stagingInfo);
    auto stagingBuffInfo = pDevice->GetMemoryViewInfo(stagingBuff);
    memcpy(stagingBuffInfo.cpuAddress, texData, imageSize);

    DebugPrint("[ASSET LOADER]", "image size: {} KB", imageSize / (1024));
    DebugPrint("[ASSET LOADER]", "image format: {}", kTexture->vkFormat);

    // may cause UB if it's not correct
    Format format = Core::FromVkFormat((VkFormat)(kTexture->vkFormat));

    uint64_t mainMipOffset;
    ktxTexture2_GetImageOffset(kTexture, 0, 0, 0, &mainMipOffset);

    // create long resource
    // TODO: impl kTexture->numLevels / mipLevels
    TextureViewDesc texDesc = {
        texWidth, texHeight, kTexture->baseDepth, format, 1, kTexture->numLayers, kTexture->isCubemap,
    };
    auto tex = pDevice->CreateTextureView(texDesc);

    auto cmd = pDevice->AllocateCommandBuffer(commandAllocator);

    // create copies for the first mipmap, next blit levels
    MemoryToTextureCopy copy = {
        .srcMemory = stagingBuff,
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
    pDevice->Submit(cmdDesc);

    // here the host waits; delete it for future async impls
    pDevice->WaitFor(QueueType::Copy);

    pDevice->Destroy(stagingBuff);
    ktxTexture2_Destroy(kTexture);

    DebugPrint("[ASSET LOADER]", "texture: {} loaded successfully", path);

    return tex;
}

auto AssetLoader::LoadGltfModel(Device* pDevice, std::string_view pathStr) -> ModelGeometry
{
    auto path = std::filesystem::path(pathStr);
    if (!std::filesystem::exists(path)) [[unlikely]]
    {
        throw std::filesystem::filesystem_error("file does not exist", path, std::make_error_code(std::errc::no_such_file_or_directory));
    }

    auto parser = fastgltf::Parser();
    auto gltfData = fastgltf::GltfDataBuffer::FromPath(path);

    auto asset = parser.loadGltf(gltfData.get(), path.parent_path(), fastgltf::Options::None);

    auto meshes = std::vector<MeshInfo>();
    auto primitives = std::vector<PrimitiveInfo>();
    auto posVec = std::vector<float3>();
    auto indexVec = std::vector<uint32_t>();

    int meshCount = 0;
    int totalPrims = 0;
    for (auto& mesh : asset->meshes)
    {
        int primCount = 0;
        for (auto& prim : mesh.primitives)
        {
            auto* posIt = prim.findAttribute("POSITION");
            // auto* norIt = prim.findAttribute("NORMAL");
            // auto* tanIt = prim.findAttribute("TANGENT");
            // auto* uvsIt = prim.findAttribute("TEXCOORD_0");

            auto& posAcc = asset->accessors[posIt->accessorIndex];
            // auto& norAcc = asset->accessors[norIt->accessorIndex];
            // auto& tanAcc = asset->accessors[tanIt->accessorIndex];
            // auto& uvsAcc = asset->accessors[uvsIt->accessorIndex];
            auto& idxAcc = asset->accessors[prim.indicesAccessor.value()];

            auto baseVertexIdx = posVec.size();
            auto baseIndexIdx = indexVec.size();

            posVec.resize(baseVertexIdx + posAcc.count);
            indexVec.resize(baseIndexIdx + idxAcc.count);

            PrimitiveInfo prim = { baseVertexIdx, posAcc.count, baseIndexIdx, idxAcc.count };
            primitives.push_back(prim);

            fastgltf::iterateAccessorWithIndex<float3>(asset.get(), posAcc, [&](float3 pos, size_t idx) {
                posVec[baseVertexIdx + idx] = pos;
                });

            // fastgltf::iterateAccessorWithIndex<float3>(asset.get(), norAcc, [&](float3 normal, size_t idx) {
            //     vertexVec[baseVertexIdx + idx].normal = normal;
            //     });

            // fastgltf::iterateAccessorWithIndex<float4>(asset.get(), tanAcc, [&](float4 tang, size_t idx) {
            //     vertexVec[baseVertexIdx + idx].tangent = tang;
            //     });

            // fastgltf::iterateAccessorWithIndex<float2>(asset.get(), uvsAcc, [&](float2 uv, size_t idx) {
            //     vertexVec[baseVertexIdx + idx].texCoord0 = uv;
            //     });

            fastgltf::iterateAccessorWithIndex<uint32_t>(asset.get(), idxAcc, [&](uint32_t index, size_t idx) {
                indexVec[baseIndexIdx + idx] = index;
                });

            ++primCount;
        }
        MeshInfo mesh;
        mesh.primitiveOffset = totalPrims;
        mesh.primitiveCount = primCount;
        meshes.push_back(mesh);
        totalPrims += primCount;
        ++meshCount;
    }

    // auto mvMeshes = pDevice->CreateMemoryView({ sizeof(MeshInfo) * meshes.size(), true });
    // auto mvPrimitives = pDevice->CreateMemoryView({ sizeof(PrimitiveInfo) * primitives.size(), true });
    auto mvPosB = pDevice->CreateMemoryView({ sizeof(float3) * posVec.size(), true });
    auto mvIndexB = pDevice->CreateMemoryView({ sizeof(uint32_t) * indexVec.size(), true });

    // auto mviMeshes = pDevice->GetMemoryViewInfo(mvMeshes);
    // auto mviPrimitives = pDevice->GetMemoryViewInfo(mvPrimitives);
    auto mviPosB = pDevice->GetMemoryViewInfo(mvPosB);
    auto mviIndexB = pDevice->GetMemoryViewInfo(mvIndexB);

    // memcpy(mviMeshes.cpuAddress, meshes.data(), sizeof(MeshInfo) * meshes.size());
    // memcpy(mviPrimitives.cpuAddress, primitives.data(), sizeof(PrimitiveInfo) * primitives.size());
    memcpy(mviPosB.cpuAddress, posVec.data(), sizeof(float3) * posVec.size());
    memcpy(mviIndexB.cpuAddress, indexVec.data(), sizeof(uint32_t) * indexVec.size());

    return { mvPosB, mvIndexB, primitives, meshes };
}