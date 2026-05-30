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
    device = &desc.device;
    // initialize buffers / memory views
}

void SurfaceImpl::Destroy()
{

}

void SurfaceImpl::SetRenderTarget(TextureView tex)
{
    dstTexture = tex;
    auto info = device->GetResourceInfo(dstTexture);
    dstHeight = info.height;
    dstWidth = info.width;
}