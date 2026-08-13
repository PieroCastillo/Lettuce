#include <filesystem>
#include <memory>

#include "Lettuce/Lettuce.hpp"
#include "common.hpp"
#include "EditorViewport.hpp"

using namespace Editor;

EditorViewport::EditorViewport(Window& window, Device& dev, Swapchain swapchain) : device(dev)
{
    pickInstance = GpuUniquePtr<uint32_t>(device);

    DescriptorTableDesc descriptorTableDesc = { 4,4,4 };
    descriptorTable = device.CreateDescriptorTable(descriptorTableDesc);

    Debug::DebugPassDesc debugPassDesc = {
        .device = device,
        .descriptorTable = descriptorTable,
        .maxCulledInstances = 10,
        .colorOutputFormat = device.GetRenderTargetFormat(swapchain),
    };
    debugPass = std::make_unique<Debug::DebugPass>(debugPassDesc);

    auto [width, height] = window.GetSize();
    Resize(width, height);
}

EditorViewport::~EditorViewport()
{
    if (pickTarget.generation != 0)
        device.Destroy(pickTarget);

    debugPass.reset();
    device.Destroy(descriptorTable);
    pickInstance.reset();
}

void EditorViewport::Resize(uint32_t width, uint32_t height)
{
    if (pickTarget.generation != 0)
        device.Destroy(pickTarget);

    TextureViewDesc pickDesc = {
        .width = width,
        .height = height,
        .depth = 1,
        .format = Format::Atomic_R32_UInt,
        .mipCount = 1,
        .layerCount = 1,
        .cpuVisible = false,
    };
    pickTarget = device.CreateTextureView(pickDesc);
}

void EditorViewport::Record(const RenderInfo& renderInfo, SceneWorkspace& scene)
{
    Debug::DebugPassRecordDesc record = {
        .fbWidth = renderInfo.fbWidth,
        .fbHeight = renderInfo.fbHeight,
        .sceneViewData = scene.GetViewData(),
        .positions = scene.GetView().GetPositionsView(),
        .indices = scene.GetView().GetIndicesView(),
        .clusters = scene.GetView().GetClustersView(),
        .meshes = scene.GetView().GetMeshesView(),
        .culledInstances = scene.GetView().GetInstanceTable(),
        .rtColorOutput = renderInfo.frame,
        .rtDepth = renderInfo.depthTarget,
        .rtPick = pickTarget,
    };
    debugPass->Record(renderInfo.cmd, record);
}

void EditorViewport::RequestPick(CommandBuffer& cmd, uint32_t x, uint32_t y)
{
    Lettuce::Utils::Algorithm::LazyCopyPixel<uint32_t>(cmd, pickTarget, x, y, pickInstance);
}