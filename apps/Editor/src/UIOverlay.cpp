
#include "Lettuce/Lettuce.hpp"
#include "common.hpp"
#include "UIOverlay.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;
using namespace Editor;

void UIOverlay::createResources()
{
    LayoutDesc layoutDesc = {};

    uint32_t width = 1366;
    uint32_t height = 768;
    uint32_t anchor = 200;

    layoutDesc.position = { 0, 0 };
    layoutDesc.scale = { anchor, height };
    squareLayout = surface->CreateLayout(layoutDesc);

    layoutDesc.position = { anchor, height - anchor };
    layoutDesc.scale = { width - (2 * anchor), anchor };
    circleLayout = surface->CreateLayout(layoutDesc);

    layoutDesc.position = { width - anchor, 0 };
    layoutDesc.scale = { anchor, height };
    roundRectLayout = surface->CreateLayout(layoutDesc);

    ImplicitGeometryDesc noCorners = {};
    square = surface->CreateGeometry(noCorners);
    circle = surface->CreateGeometry(noCorners);
    roundRect = surface->CreateGeometry(noCorners);

    redBrush = surface->CreateBrush({ .color = Colors::Red });
    blueBrush = surface->CreateBrush({ .color = Colors::Blue });
    yellowBrush = surface->CreateBrush({ .color = Colors::Yellow });
}

UIOverlay::UIOverlay(Device& device, Swapchain swapchain)
{
    SurfaceDesc surfaceCI = {
        .device = device,
        .maxImplicitGeometries = 10000,
        .maxBrushes = 10000,
        .maxDrawCommands = 10000,
        .colorOutputFormat = device.GetRenderTargetFormat(swapchain),
    };
    surface = std::make_unique<Surface>(surfaceCI);

    createResources();
}

UIOverlay::~UIOverlay()
{
    surface.reset();
}

void UIOverlay::Record(RenderInfo& renderInfo)
{
    auto scmd = SurfaceCommandBuffer(*surface, renderInfo.cmd);
    scmd.Draw(3, square, blueBrush, squareLayout);
    scmd.Draw(2, circle, redBrush, circleLayout);
    scmd.Draw(1, roundRect, yellowBrush, roundRectLayout);
    scmd.DrawSurface({ renderInfo.frame, renderInfo.depthTarget, renderInfo.pickTarget, { 0, 0, renderInfo.fbWidth, renderInfo.fbHeight } });
}