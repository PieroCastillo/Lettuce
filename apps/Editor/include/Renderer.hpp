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
        auto GetCommandAllocator() const { return cmdAlloc; }
    };
};
#endif // EDITOR_RENDERER_HPP