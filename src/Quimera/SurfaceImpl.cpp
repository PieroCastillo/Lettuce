// standard headers
#include <iostream>
#include <fstream>
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

    std::vector<uint32_t> shadersBuffer;
    auto path = std::string("./src/Quimera/comp.surface.spv");
    auto shadersFile = std::ifstream(path, std::ios::ate | std::ios::binary);
    if (!shadersFile) throw std::runtime_error(path + " does not exist");
    auto fileSize = (uint32_t)shadersFile.tellg();
    shadersBuffer.resize(fileSize / sizeof(uint32_t));
    shadersFile.seekg(0);
    shadersFile.read((char*)shadersBuffer.data(), fileSize);

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

    auto shaderBin = pDevice->CreateShader({ shadersBuffer });
    pPrepare = pDevice->CreatePipeline({ "pPrepareMain", shaderBin, dtSurface });
    pTileBinning = pDevice->CreatePipeline({ "pTileBinningMain", shaderBin, dtSurface });
    pBrushes = pDevice->CreatePipeline({ "pBrushesMain", shaderBin, dtSurface });

    pDevice->Destroy(shaderBin);
}

void SurfaceImpl::Destroy()
{
    pDevice->Destroy(pBrushes);
    pDevice->Destroy(pTileBinning);
    pDevice->Destroy(pPrepare);
    pDevice->Destroy(dtSurface);
    pDevice->Destroy(bBrushes.mv);
    pDevice->Destroy(bImplicitGeometries.mv);
    pDevice->Destroy(bTransforms.mv);
    pDevice->Destroy(bDrawCommands.mv);
}