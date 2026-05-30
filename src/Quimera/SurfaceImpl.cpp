// standard headers
#include <memory>
#include <memory_resource>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/SurfaceImpl.hpp"
#include "Lettuce/Quimera/api.hpp"

using namespace Lettuce::Quimera;
using namespace Lettuce::Core;

void SurfaceImpl::Create(const SurfaceDesc& desc)
{
    pDevice = &desc.device;

    // initialize buffers / memory views
    bDrawCommands.mv = pDevice->CreateMemoryView({ desc.maxDrawCommands * sizeof(DrawCommand), true });
    bDrawCommands.maxCount = desc.maxDrawCommands;
    bDrawCommands.offset = 0;
    bDrawCommands.addr = pDevice->GetMemoryViewInfo(bDrawCommands.mv).cpuAddress;

    bTransforms.mv = pDevice->CreateMemoryView({ desc.maxDrawCommands * sizeof(float3x3), true });
    bTransforms.maxCount = desc.maxDrawCommands;
    bTransforms.offset = 0;
    bTransforms.addr = pDevice->GetMemoryViewInfo(bTransforms.mv).cpuAddress;

    bImplicitGeometries.mv = pDevice->CreateMemoryView({ desc.maxImplicitGeometries * sizeof(ImplicitGeometryStorage), true });
    bImplicitGeometries.maxCount = desc.maxImplicitGeometries;
    bImplicitGeometries.offset = 0;
    bImplicitGeometries.addr = pDevice->GetMemoryViewInfo(bImplicitGeometries.mv).cpuAddress;

    bBrushes.mv = pDevice->CreateMemoryView({ desc.maxBrushes * sizeof(BrushStorage), true });
    bBrushes.maxCount = desc.maxBrushes;
    bBrushes.offset = 0;
    bBrushes.addr = pDevice->GetMemoryViewInfo(bBrushes.mv).cpuAddress;

    dtSurface = pDevice->CreateDescriptorTable({ 4,4,4 });
}

void SurfaceImpl::Destroy()
{
    pDevice->Destroy(dtSurface);
    pDevice->Destroy(bBrushes.mv);
    pDevice->Destroy(bImplicitGeometries.mv);
    pDevice->Destroy(bTransforms.mv);
    pDevice->Destroy(bDrawCommands.mv);
}

void SurfaceImpl::SetRenderTarget(TextureView tex)
{
    dstTexture = tex;
    auto info = pDevice->GetResourceInfo(dstTexture);
    dstHeight = info.height;
    dstWidth = info.width;
}