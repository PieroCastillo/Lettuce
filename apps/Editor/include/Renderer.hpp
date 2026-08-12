/*
Created by @PieroCastillo on 2026-08-11
*/
#ifndef EDITOR_RENDERER_HPP
#define EDITOR_RENDERER_HPP

#include <chrono>
#include <cstdint>
#include <span>
#include <string>

#include "Lettuce/Lettuce.hpp"
#include "Window.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;

namespace Editor
{
    class Renderer
    {
    private:
        std::unique_ptr<Device> device;

        Swapchain swapchain;
        CommandAllocator cmdAlloc;

        TextureView depthTarget;
        TextureView pickTarget;
    public:
        Renderer(Window& window);
        ~Renderer();

        void BeginFrame();
        void EndFrame();

        void RenderScene(SceneView&);

        void Resize(uint32_t width, uint32_t height);

        auto GetDevice() -> Device&{ return *device;};
        auto GetSwapchain() const { return swapchain; };

        auto GetDepthTarget() const { return depthTarget; }
        auto GetPickTarget() const { return pickTarget; }
        auto GetCommandAllocator() const { return cmdAlloc; }
    };

    struct RenderInfo
    {
        CommandBuffer& cmd;
        TextureView frame;
        TextureView depthTarget;
        TextureView pickTarget;
        uint32_t fbWidth;
        uint32_t fbHeight; 
    };
};
#endif // EDITOR_RENDERER_HPP