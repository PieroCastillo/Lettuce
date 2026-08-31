#include "Lettuce/Lettuce.hpp"
#include "glfw/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef CreateFont

#include <algorithm>
#include <atomic>
#include <bit>
#include <filesystem>
#include <fstream>
#include <memory>
#include <print>
#include <string>
#include <system_error>
#include <vector>

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;

GLFWwindow* window;

uint32_t width = 1366;
uint32_t height = 768;

std::unique_ptr<Device> device;
std::unique_ptr<Surface> surface;

Swapchain swapchain;
CommandAllocator cmdAlloc;
TextureView tDepthTarget;
TextureView tPickTarget;

Layout squareLayout;
Layout circleLayout;
Layout roundRectLayout;
Geometry square;
Geometry circle;
Geometry roundRect;
Brush redBrush;
Brush blueBrush;
Brush yellowBrush;

Font fontFiraCode;
std::vector<Layout> textBaseLayouts;
Brush whiteBrush;
std::vector<Glyph> textGlyphs;

void draw2dScene(CommandBuffer& lcmd, TextureView frame, uint32_t fbWidth, uint32_t fbHeight)
{
    auto cmd = SurfaceCommandBuffer(*surface, lcmd);
    cmd.Draw(4, square, blueBrush, squareLayout);
    cmd.Draw(3, circle, redBrush, circleLayout);
    cmd.Draw(2, roundRect, yellowBrush, roundRectLayout);
    for (auto baseLayout : textBaseLayouts)
        cmd.Draw(1, textGlyphs, whiteBrush, baseLayout);
    cmd.DrawSurface({ frame, tDepthTarget, tPickTarget, { 0, 0, fbWidth, fbHeight } });
}

void create2dResources()
{
    LayoutDesc layoutDesc = {};

    layoutDesc.position = { 300, 300 };
    layoutDesc.scale = { 300, 300 };
    squareLayout = surface->CreateLayout(layoutDesc);

    layoutDesc.position = { 520, 150 };
    layoutDesc.scale = { 300, 300 };
    circleLayout = surface->CreateLayout(layoutDesc);

    layoutDesc.position = { 600, 420 };
    layoutDesc.scale = { 250, 150 };
    roundRectLayout = surface->CreateLayout(layoutDesc);

    ImplicitGeometryDesc squareData = {};
    square = surface->CreateGeometry(squareData);

    ImplicitGeometryDesc circleData = {
        0.5, 0.5,0.5, 0.5,
    };
    circle = surface->CreateGeometry(circleData);

    ImplicitGeometryDesc yellowRoundRectData = {
        0.1, 0.1, 0.1, 0.1,
    };
    roundRect = surface->CreateGeometry(yellowRoundRectData);

    redBrush = surface->CreateBrush({ .color = Colors::Red });
    blueBrush = surface->CreateBrush({ .color = Colors::Blue });
    yellowBrush = surface->CreateBrush({ .color = Colors::Yellow });
    // prepare for copy ops
    auto copyCmdAlloc = device->CreateCommandAllocator({ QueueType::Copy });

    // read font
    std::filesystem::path fontPath = "../../../../samples/assets/FiraCode-Regular.ttf";

    std::ifstream fontFile(fontPath, std::ios::binary | std::ios::ate);
    if (!fontFile) throw std::runtime_error(fontPath.string() + " does not exist");

    auto size = fontFile.tellg();
    auto fontData = std::vector<uint8_t>(size);

    fontFile.seekg(0);
    fontFile.read((char*)fontData.data(), size);

    fontFiraCode = surface->CreateFont({ fontData });
    textGlyphs = Lettuce::Utils::GlyphLoader::ShapeText(surface.get(), fontFiraCode, "hello world!  abd xyz qwerty ===>");

    // load utf8 symbols
    auto alphabetGlyphsIDs = std::vector<uint32_t>();
    for (const auto& g : textGlyphs)
        alphabetGlyphsIDs.push_back(g.glyphID);

    surface->LoadGlyphs(copyCmdAlloc, fontFiraCode, alphabetGlyphsIDs);

    textBaseLayouts.resize(3);

    layoutDesc.position = { 200, 300 };
    layoutDesc.scale = { 14, 14 };
    textBaseLayouts[0] = surface->CreateLayout(layoutDesc); // little

    layoutDesc.position = { 200, 400 };
    layoutDesc.scale = { 28, 28 };
    textBaseLayouts[1] = surface->CreateLayout(layoutDesc); // medium

    layoutDesc.position = { 200, 500 };
    layoutDesc.scale = { 40, 40 };
    textBaseLayouts[2] = surface->CreateLayout(layoutDesc); // big

    whiteBrush = surface->CreateBrush({ .color = Colors::White });

    device->Destroy(copyCmdAlloc);

    std::println("glyph count: {}", textGlyphs.size());
    for (const auto& g : textGlyphs)
    {
        std::println("glyphID: {}, relOffX: {}, relOffY: {}", g.glyphID, std::bit_cast<float>(g.offsetX), std::bit_cast<float>(g.offsetY));
    }
}

void cleanup2dResources()
{
    surface->Destroy(fontFiraCode);
}

uint32_t oldFbWidth = width;
uint32_t oldFbHeight = height;
void mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, (int*)&width, (int*)&height);

        if (width == 0 || height == 0)
        {
            glfwWaitEvents();
            continue;
        }

        auto fbSize = device->NextFrame(swapchain);

        if (fbSize.width != oldFbWidth || fbSize.height != oldFbHeight) [[unlikely]]
        {
            device->WaitFor(QueueType::Graphics);
            device->Destroy(tDepthTarget);

            RenderTargetDesc depthDesc = {
                .width = width,
                .height = height,
                .type = RenderTargetType::Depth_D32,
                .defaultClearValue = DepthStencilClear {1.0f, 0},
            };
            tDepthTarget = device->CreateTextureView(depthDesc);

            if (tPickTarget.generation != 0)
                device->Destroy(tPickTarget);

            RenderTargetDesc pickDesc = {
                .width = width,
                .height = height,
                .type = RenderTargetType::Depth_D32,
                .defaultClearValue = ColorClear{},
            };
            tPickTarget = device->CreateTextureView(pickDesc);

            oldFbWidth = fbSize.width;
            oldFbHeight = fbSize.height;
        }

        device->Reset(cmdAlloc);
        auto frame = device->GetCurrentRenderTarget(swapchain);
        auto cmd = device->AllocateCommandBuffer(cmdAlloc);

        BarrierDesc bClearColor = {
            PipelineAccess::Write,
            PipelineStage::Clear,
            PipelineAccess::Write,
            PipelineStage::ColorAttachmentOutput,
        };

        auto color = ColorClear{ 0.498, 0.498, 0.498, 1.0 };
        // cmd.ClearTexture({ frame, color,0, 1, 0, 1 });

        // cmd.Barrier({ bClearColor });

        draw2dScene(cmd, frame, fbSize.width, fbSize.height);

        std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

        CommandBufferSubmitDesc submitDesc = {
            .queueType = QueueType::Graphics,
            .commandBuffers = std::span(cmds),
            .presentSwapchain = swapchain,
        };

        device->Submit(submitDesc);
        device->DisplayFrame(swapchain);
        device->WaitFor(QueueType::Graphics);

        glfwPollEvents();
    }
}

void initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, "Lettuce Quimera 2D Test", NULL, NULL);
}
void cleanupWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void initLettuce()
{
    auto hwnd = glfwGetWin32Window(window);
    auto hmodule = GetModuleHandle(NULL);

    DeviceDesc deviceCI = {
        .preferDedicated = true,
    };
    device = std::make_unique<Device>(deviceCI);

    SwapchainDesc swapchainDesc = {
        .clipped = true,
        .windowPtr = &hwnd,
        .applicationPtr = &hmodule,
    };
    swapchain = device->CreateSwapchain(swapchainDesc);

    SurfaceDesc surfaceCI = {
        .device = *device,
        .maxImplicitGeometries = 10000,
        .maxBrushes = 10000,
        .maxDrawCommands = 10000,
        .colorOutputFormat = device->GetRenderTargetFormat(swapchain),
    };
    surface = std::make_unique<Surface>(surfaceCI);

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Graphics,
    };
    cmdAlloc = device->CreateCommandAllocator(cmdAllocDesc);

    RenderTargetDesc depthDesc = {
        .width = width,
        .height = height,
        .type = RenderTargetType::Depth_D32,
        .defaultClearValue = DepthStencilClear {1.0f, 0},
    };
    tDepthTarget = device->CreateTextureView(depthDesc);

    RenderTargetDesc pickDesc = {
        .width = width,
        .height = height,
        .type = RenderTargetType::ColorRGB_R32UInt,
        .defaultClearValue = ColorClear{},
    };
    tPickTarget = device->CreateTextureView(pickDesc);
}

void cleanupLettuce()
{
    surface.reset();
    device->Destroy(tDepthTarget);
    device->Destroy(tPickTarget);
    device->Destroy(cmdAlloc);
    device->Destroy(swapchain);
    device.reset();
}

int main()
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    initWindow();
    initLettuce();
    create2dResources();
    mainLoop();
    device->WaitFor(QueueType::Graphics);
    cleanup2dResources();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}