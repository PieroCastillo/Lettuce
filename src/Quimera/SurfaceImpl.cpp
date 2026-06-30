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
    mvSurfaceData =  pDevice->CreateMemoryView({sizeof(SurfaceData), true});
    mvSurfaceDataPtr = (SurfaceData*)pDevice->GetMemoryViewInfo(mvSurfaceData).cpuAddress;
    bDrawCommands = Buffer<DrawCommand>(pDevice, desc.maxDrawCommands);
    bLayouts = Buffer<LayoutStorage>(pDevice, desc.maxDrawCommands);
    bImplicitGeometry = Buffer<ImplicitGeometryStorage>(pDevice, desc.maxImplicitGeometries);
    bSolidColorBrush = Buffer<SolidColorBrushStorage>(pDevice, desc.maxBrushes);

    dtSurface = pDevice->CreateDescriptorTable({ 4,4,4 });

    auto shaderBin = pDevice->CreateShader({ shadersBuffer });
    pDrawCommands = pDevice->CreatePipeline({ "pDrawCommandsMain", shaderBin, dtSurface });

    pDevice->Destroy(shaderBin);
}

void SurfaceImpl::Destroy()
{
    pDevice->Destroy(pDrawCommands);
    pDevice->Destroy(dtSurface);
    pDevice->Destroy(bDrawCommands.mv);
    pDevice->Destroy(bLayouts.mv);
    pDevice->Destroy(bImplicitGeometry.mv);
    pDevice->Destroy(bSolidColorBrush.mv);
    pDevice->Destroy(mvSurfaceData);
}