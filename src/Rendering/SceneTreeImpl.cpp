// standard headers
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <memory_resource>
#include <span>
#include <string>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Rendering/SceneTree.hpp"
#include "Lettuce/Rendering/SceneTreeImpl.hpp"

using namespace Lettuce::Rendering;
using namespace Lettuce::Core;

void SceneTreeImpl::Create(const SceneTreeDesc& desc)
{
    device = &desc.device;

    // load spv
    std::vector<uint32_t> shadersBuffer;
    auto path = std::string("./comp.sceneTree.spv");
    auto shadersFile = std::ifstream(path, std::ios::ate | std::ios::binary);
    if (!shadersFile) throw std::runtime_error(path + " does not exist");
    auto fileSize = (uint32_t)shadersFile.tellg();
    shadersBuffer.resize(fileSize / sizeof(uint32_t));
    shadersFile.seekg(0);
    shadersFile.read((char*)shadersBuffer.data(), fileSize);

    // creates descriptor table & pipelines
    dtSceneTree = device->CreateDescriptorTable({ 2,2,2 });

    auto shaderBin = device->CreateShader({ .bytecode = shadersBuffer, });
    pBuildGeometryCluster = device->CreatePipeline({ "buildCluster", shaderBin, dtSceneTree });
    pBuildScenePartition = device->CreatePipeline({ "buildPartition", shaderBin, dtSceneTree });
    pUpdateScenePartition = device->CreatePipeline({ "updatePartition", shaderBin, dtSceneTree });
    device->Destroy(shaderBin);
}

void SceneTreeImpl::Destroy()
{
    device->Destroy(pUpdateScenePartition);
    device->Destroy(pBuildScenePartition);
    device->Destroy(pBuildGeometryCluster);
    device->Destroy(dtSceneTree);
}