// standard headers
#include <fstream>
#include <filesystem>
#include <limits>
#include <memory>
#include <print>
#include <span>
#include <system_error>
#include <vector>

// external headers
#include "glfw/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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
#include "Lettuce/Rendering/SceneTree.hpp"

using namespace Lettuce::Utils;
using namespace Lettuce::Rendering;

// template<> struct fastgltf::ElementTraits<float2> : fastgltf::ElementTraitsBase<float2, AccessorType::Vec2, float> {};
// template<> struct fastgltf::ElementTraits<float3> : fastgltf::ElementTraitsBase<float3, AccessorType::Vec3, float> {};
// template<> struct fastgltf::ElementTraits<float4> : fastgltf::ElementTraitsBase<float4, AccessorType::Vec4, float> {};
// template<> struct fastgltf::ElementTraits<uint32_t> : fastgltf::ElementTraitsBase<uint32_t, AccessorType::Scalar, uint32_t> {};

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
    auto token = pDevice->SubmitAsync(cmdDesc);
    pDevice->WaitFor(token);

    pDevice->Destroy(stagingBuff);
    ktxTexture2_Destroy(kTexture);

    DebugPrint("[ASSET LOADER]", "texture: {} loaded successfully", path);

    return tex;
}

auto AssetLoader::LoadGltfModel(Device* pDevice, std::string_view pathStr) -> ModelGeometry
{
    /* LOAD GEOMETRY */
    auto path = std::filesystem::path(pathStr);
    if (!std::filesystem::exists(path)) [[unlikely]]
    {
        throw std::filesystem::filesystem_error("file does not exist", path, std::make_error_code(std::errc::no_such_file_or_directory));
    }

    auto parser = fastgltf::Parser();
    auto gltfData = fastgltf::GltfDataBuffer::FromPath(path);

    auto asset = parser.loadGltf(gltfData.get(), path.parent_path(), fastgltf::Options::None);

    /* GET PRIMITIVES*/
    struct PrimitiveInfo
    {
        uint32_t vertexOffset;
        uint32_t vertexCount;
        uint32_t indexOffset;
        uint32_t indexCount;
    };

    struct MeshInfo
    {
        uint32_t primitiveOffset;
        uint32_t primitiveCount;
    };

    auto meshes = std::vector<MeshInfo>();
    auto primitives = std::vector<PrimitiveInfo>();
    auto tempVerts = std::vector<VertexL>();

    // temp index stream
    auto indexVec = std::vector<uint32_t>();

    uint32_t meshCount = 0;
    uint32_t totalPrims = 0;
    for (auto& mesh : asset->meshes)
    {
        uint32_t primCount = 0;

        for (auto& prim : mesh.primitives)
        {
            auto* posIt = prim.findAttribute("POSITION");
            auto* norIt = prim.findAttribute("NORMAL");
            auto* uvsIt = prim.findAttribute("TEXCOORD_0");

            auto& posAcc = asset->accessors[posIt->accessorIndex];
            auto& norAcc = asset->accessors[norIt->accessorIndex];
            auto& uvsAcc = asset->accessors[uvsIt->accessorIndex];
            auto& idxAcc = asset->accessors[prim.indicesAccessor.value()];

            auto baseVertexIdx = tempVerts.size();
            auto baseIndexIdx = indexVec.size();

            tempVerts.resize(baseVertexIdx + posAcc.count);
            indexVec.resize(baseIndexIdx + idxAcc.count);

            primitives.push_back({
                static_cast<uint32_t>(baseVertexIdx),
                static_cast<uint32_t>(posAcc.count),
                static_cast<uint32_t>(baseIndexIdx),
                static_cast<uint32_t>(idxAcc.count)
                });

            fastgltf::iterateAccessorWithIndex<float3>(asset.get(), posAcc, [&](float3 pos, size_t idx)
                {
                    tempVerts[baseVertexIdx + idx].pos = pos;
                });

            fastgltf::iterateAccessorWithIndex<uint32_t>(asset.get(), idxAcc, [&](uint32_t index, size_t idx)
                {
                    indexVec[baseIndexIdx + idx] = index;
                });

            fastgltf::iterateAccessorWithIndex<float3>(asset.get(), norAcc, [&](float3 normal, size_t idx)
                {
                    tempVerts[baseVertexIdx + idx].norm = normal;
                });

            fastgltf::iterateAccessorWithIndex<float2>(asset.get(), uvsAcc, [&](float2 uv, size_t idx)
                {
                    tempVerts[baseVertexIdx + idx].texCoord0 = uv;
                });

            ++primCount;
        }

        meshes.push_back({ totalPrims, primCount });
        totalPrims += primCount;
        ++meshCount;
    }
    DebugPrint("[ASSET LOADER]", "{} parsed successfully", path.filename().string());

    /* CONVERT TO MESHLETS */
    constexpr uint32_t maxVertices = 64;
    constexpr uint32_t minTriangles = 32;
    constexpr uint32_t maxTriangles = 126;
    constexpr float coneWeight = 0.5f;

    // output
    ModelGeometry outModel = {};

    uint32_t geometryIdx = 0;
    uint32_t clusterIdx = 0;

    for (auto& mesh : meshes)
    {
        auto firstCluster = clusterIdx;

        auto meshAabbMin = float3(std::numeric_limits<float>::max());
        auto meshAabbMax = float3(std::numeric_limits<float>::lowest());

        for (uint32_t primIdx = mesh.primitiveOffset;
            primIdx < mesh.primitiveOffset + mesh.primitiveCount;
            ++primIdx)
        {
            auto& prim = primitives[primIdx];

            auto maxMeshlets = meshopt_buildMeshletsBound(prim.indexCount, maxVertices, maxTriangles);

            auto meshlets = std::vector<meshopt_Meshlet>(maxMeshlets);
            auto meshletVerts = std::vector<uint32_t>(prim.indexCount);
            auto meshletTris = std::vector<uint8_t>(prim.indexCount);

            auto meshletCount = meshopt_buildMeshletsSpatial(
                meshlets.data(),
                meshletVerts.data(),
                meshletTris.data(),
                &indexVec[prim.indexOffset],
                prim.indexCount,
                &tempVerts[prim.vertexOffset].pos.x,
                prim.vertexCount,
                sizeof(VertexL),
                maxVertices,
                minTriangles,
                maxTriangles,
                coneWeight);

            meshlets.resize(meshletCount);

            for (auto& meshlet : meshlets)
            {
                auto aabbMin = float3(std::numeric_limits<float>::max());
                auto aabbMax = float3(std::numeric_limits<float>::lowest());

                uint32_t globalVertexOffset = static_cast<uint32_t>(outModel.vertexTable.size());
                uint32_t globalTriangleOffset = static_cast<uint32_t>(outModel.trianglesTable.size());

                for (uint32_t v = 0; v < meshlet.vertex_count; ++v)
                {
                    auto localVertex = meshletVerts[meshlet.vertex_offset + v];
                    auto& vertex = tempVerts[prim.vertexOffset + localVertex];

                    outModel.vertexTable.push_back(vertex);

                    aabbMin = { std::min(aabbMin.x, vertex.pos.x), std::min(aabbMin.y, vertex.pos.y), std::min(aabbMin.z, vertex.pos.z) };
                    aabbMax = { std::max(aabbMax.x, vertex.pos.x), std::max(aabbMax.y, vertex.pos.y), std::max(aabbMax.z, vertex.pos.z) };
                }

                auto triangleCount = meshlet.triangle_count * 3;
                for (uint32_t t = 0; t < triangleCount; ++t)
                {
                    outModel.trianglesTable.push_back(meshletTris[meshlet.triangle_offset + t]);
                }

                meshAabbMin = { std::min(meshAabbMin.x, aabbMin.x), std::min(meshAabbMin.y, aabbMin.y), std::min(meshAabbMin.z, aabbMin.z) };
                meshAabbMax = { std::max(meshAabbMax.x, aabbMax.x), std::max(meshAabbMax.y, aabbMax.y), std::max(meshAabbMax.z, aabbMax.z) };

                outModel.clusterBuilds.push_back({
                    .bboxMin = aabbMin,
                    .bboxMax = aabbMax,
                    .cone = {},
                    .lodError = 0.0f,
                    .clusterID = clusterIdx,
                    .geometryIndex = geometryIdx,
                    .triangleOffset = globalTriangleOffset,
                    .triangleCount = meshlet.triangle_count,
                    .vertexOffset = globalVertexOffset,
                    .vertexCount = meshlet.vertex_count,
                    .vertexStride = sizeof(VertexL),
                    });

                ++clusterIdx;
            }
        }

        outModel.meshes.push_back({ meshAabbMin, meshAabbMax, firstCluster, clusterIdx - firstCluster });

        ++geometryIdx;
    }

    /*TODO: get scene */

    DebugPrint("[ASSET LOADER STATISTICS]", "\n Mesh count: {}\n Cluster count: {}\n Triangle count: {}",
        outModel.meshes.size(), outModel.clusterBuilds.size(), outModel.trianglesTable.size());

    return std::move(outModel);
}

auto AssetLoader::LoadGtlfAsGeometry(Device* pDevice, std::string_view pathStr) -> GeometrySource
{
    /* LOAD GEOMETRY */
    auto path = std::filesystem::path(pathStr);
    if (!std::filesystem::exists(path)) [[unlikely]]
    {
        throw std::filesystem::filesystem_error("file does not exist", path, std::make_error_code(std::errc::no_such_file_or_directory));
    }

    auto parser = fastgltf::Parser();
    auto gltfData = fastgltf::GltfDataBuffer::FromPath(path);
    auto asset = parser.loadGltf(gltfData.get(), path.parent_path(), fastgltf::Options::None);

    std::vector<float3> tempPosStream;
    std::vector<float3> tempNorStream;
    std::vector<float4> tempTanStream;
    std::vector<float2> tempUvsStream;
    std::vector<uint32_t> tempIdxStream;
    std::vector<meshopt_Meshlet> meshlets;
    std::vector<uint32_t> meshletVerts;
    std::vector<uint8_t> meshletTris;

    GeometrySource source;

    constexpr uint32_t maxVertices = 64;
    constexpr uint32_t minTriangles = 32;
    constexpr uint32_t maxTriangles = 126;
    constexpr float coneWeight = 0.5f;

    for (auto& mesh : asset->meshes)
    {
        const auto clustersOldSize = source.clusters.size();

        for (auto& prim : mesh.primitives)
        {
            // get attributes and accesors
            auto* posIt = prim.findAttribute("POSITION");
            auto* norIt = prim.findAttribute("NORMAL");
            auto* tanIt = prim.findAttribute("TANGENT");
            auto* uvsIt = prim.findAttribute("TEXCOORD_0");

            auto& posAcc = asset->accessors[posIt->accessorIndex];
            auto& norAcc = asset->accessors[norIt->accessorIndex];
            auto& tanAcc = asset->accessors[tanIt->accessorIndex];
            auto& uvsAcc = asset->accessors[uvsIt->accessorIndex];
            auto& idxAcc = asset->accessors[prim.indicesAccessor.value()];

            const auto posCount = posAcc.count;
            const auto norCount = norIt != prim.attributes.end() ? norAcc.count : 0;
            const auto tanCount = tanIt != prim.attributes.end() ? tanAcc.count : 0;
            const auto uvsCount = uvsIt != prim.attributes.end() ? uvsAcc.count : 0;
            const auto idxCount = idxAcc.count;

            // get sizes, resize vectors at sources and do copies
            tempPosStream.resize(posAcc.count);
            tempNorStream.resize(norAcc.count);
            tempTanStream.resize(tanAcc.count);
            tempUvsStream.resize(uvsAcc.count);
            tempIdxStream.resize(idxAcc.count);

            fastgltf::copyFromAccessor<float3>(asset.get(), posAcc, tempPosStream.data());
            if (norCount > 0) fastgltf::copyFromAccessor<float3>(asset.get(), norAcc, tempNorStream.data());
            if (tanCount > 0) fastgltf::copyFromAccessor<float3>(asset.get(), tanAcc, tempTanStream.data());
            if (uvsCount > 0) fastgltf::copyFromAccessor<float2>(asset.get(), uvsAcc, tempUvsStream.data());
            fastgltf::copyFromAccessor<uint32_t>(asset.get(), idxAcc, tempIdxStream.data());

            auto maxMeshlets = meshopt_buildMeshletsBound(idxAcc.count, maxVertices, maxTriangles);

            meshlets.resize(maxMeshlets);
            meshletVerts.resize(idxAcc.count);
            meshletTris.resize(idxAcc.count);

            auto meshletCount = meshopt_buildMeshletsSpatial(
                meshlets.data(),
                meshletVerts.data(),
                meshletTris.data(),
                tempIdxStream.data(),
                idxAcc.count,
                &tempPosStream.data()->x,
                posAcc.count,
                sizeof(float3),
                maxVertices,
                minTriangles,
                maxTriangles,
                coneWeight);
            meshlets.resize(meshletCount);

            source.positions.reserve(source.positions.size() + meshletVerts.size());
            if (norCount > 0) source.normals.reserve(source.normals.size() + meshletVerts.size());
            if (tanCount > 0) source.tangents.reserve(source.tangents.size() + meshletVerts.size());
            if (uvsCount > 0) source.texCoords0.reserve(source.texCoords0.size() + meshletVerts.size());

            source.indices.reserve(source.indices.size() + meshletTris.size());

            // copy vertices for each available stream
            for (const auto& ml : meshlets)
            {
                auto posOldSize = source.positions.size();
                auto norOldSize = source.normals.size();
                auto tanOldSize = source.tangents.size();
                auto uvsOldSize = source.texCoords0.size();
                auto idxOldSize = source.indices.size();

                // iter the range meshlet verts indices[vertex offset, vertex offset + vertex count]
                for (int i = ml.vertex_offset; i < ml.vertex_offset + ml.vertex_count; ++i)
                {
                    auto vertIdx = meshletVerts[i];
                    source.positions.push_back(tempPosStream[vertIdx]);

                    if (norCount > 0) source.normals.push_back(tempNorStream[vertIdx]);
                    if (tanCount > 0) source.tangents.push_back(tempTanStream[vertIdx]);
                    if (uvsCount > 0) source.texCoords0.push_back(tempUvsStream[vertIdx]);
                }

                // similar the previous one, but using meshlets triangles[]
                for (int idx = ml.triangle_offset; idx < ml.triangle_offset + (ml.triangle_count * 3); ++idx)
                    source.indices.push_back(meshletTris[idx]);

                auto cluster = ClusterStorage{
                    .materialHeap = 0,
                    .materialID = 0,
                    .cone = {},
                    .aabbMin = {},
                    .aabbMax = {},
                    .centroid = {},
                    .positionsBaseOffset = (uint32_t)(posOldSize),
                    .positionsCount = (uint32_t)(source.positions.size() - posOldSize),
                    .normalsBaseOffset = (uint32_t)(norOldSize),
                    .normalsCount = (uint32_t)(source.normals.size() - norOldSize),
                    .tangentsBaseOffset = (uint32_t)(tanOldSize),
                    .tangentsCount = (uint32_t)(source.tangents.size() - tanOldSize),
                    .texCoords0BaseOffset = (uint32_t)(uvsOldSize),
                    .texCoords0Count = (uint32_t)(source.texCoords0.size() - uvsOldSize),
                    .trianglesBaseOffset = (uint32_t)(idxOldSize),
                    .triangleCount = (uint32_t)(source.indices.size() - idxOldSize),
                    .clusterID = 0,
                };
                source.clusters.push_back(cluster);
            }
        }

        auto mesh = MeshStorage{
            .aabbMin = {},
            .aabbMax = {},
            .clusterOffset = (uint32_t)(clustersOldSize),
            .clusterCount = (uint32_t)(source.clusters.size() - clustersOldSize),
        };
        source.meshes.push_back(mesh);
    }

    const auto& nodesIdxs = asset->scenes.at(asset->defaultScene.value()).nodeIndices;
    const auto nodeCount = nodesIdxs.size();
    source.instances.reserve(nodeCount);

    for (auto nodeIdx : nodesIdxs)
    {
        const auto& node = asset->nodes[nodeIdx];

        auto transform = glm::identity<float4x4>();
        if (std::holds_alternative<fastgltf::math::fmat4x4>(node.transform))
        {
            const auto& m = std::get<fastgltf::math::fmat4x4>(node.transform);
            transform = glm::make_mat4(m.data());
        }
        else if (std::holds_alternative<fastgltf::TRS>(node.transform))
        {
            const auto& trs = std::get<fastgltf::TRS>(node.transform);
            transform = glm::translate(glm::mat4(1.0f), glm::vec3(trs.translation.x(), trs.translation.y(), trs.translation.z())) *
                glm::mat4_cast(glm::quat(trs.rotation.w(), trs.rotation.x(), trs.rotation.y(), trs.rotation.z())) *
                glm::scale(glm::mat4(1.0f), glm::vec3(trs.scale.x(), trs.scale.y(), trs.scale.z()));
        }

        auto inst = InstanceStorage{
            .localTransform = transform,
            .meshIdx = (uint32_t)(node.meshIndex.value_or(std::numeric_limits<size_t>::max())),
        };
        source.instances.push_back(inst);
    }

    DebugPrint("[ASSET LOADER STATISTICS]", "#(position, normal, tangent, uvs): {}, {}, {}, {}  \n #idx : {} \n #clusters : {} \n #meshes : {} \n", source.positions.size(), source.normals.size(), source.tangents.size(), source.texCoords0.size(), source.indices.size(), source.clusters.size(), source.meshes.size());

    return source;
}