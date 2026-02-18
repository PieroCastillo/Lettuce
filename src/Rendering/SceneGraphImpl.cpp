// standard headers
#include <memory>
#include <memory_resource>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Rendering/api.hpp"
#include "Lettuce/Rendering/HelperStructs.hpp"
#include "Lettuce/Rendering/SceneGraphImpl.hpp"

using namespace Lettuce::Rendering;
using namespace Lettuce::Core;

void SceneGraphImpl::Create(const SceneGraphDesc& desc)
{
    device = &(desc.device);
    allocator = desc.allocator;

    // sceneMemoryView = allocator->AllocateMemory(sizeof(SceneGPUData));
    // lightMemoryView = allocator->AllocateMemory(sizeof(LightGPUData) * maxLightsCount);

    // sceneGPUData = std::span<SceneGPUData, 1>(static_cast<SceneGPUData*>(sceneMemoryView.cpuAddress), 1);
    // lightsGPUData = std::span<LightGPUData, maxLightsCount>(static_cast<LightGPUData*>(lightMemoryView.cpuAddress), maxLightsCount);
}

void SceneGraphImpl::Destroy()
{
    allocator->ReleaseMemory(lightMemoryView);
    allocator->ReleaseMemory(sceneMemoryView);
}