// standard headers
#include <memory>
#include <print>
#include <vector>

// project headers
#include "Lettuce/Lettuce.hpp"
#include "common.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "Input.hpp"
#include "SceneWorkspace.hpp"
#include "UIOverlay.hpp"
#include "EditorViewport.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Quimera;
using namespace Lettuce::Rendering;

namespace Editor
{
    class App
    {
        std::unique_ptr<Window> window;
        std::unique_ptr<Renderer> renderer;
        InputSystem input;

        std::unique_ptr<SceneWorkspace> workspace;
        std::unique_ptr<UIOverlay> uiOverlay;
        std::unique_ptr<EditorViewport> viewport;

        Lettuce::Utils::FrameTimer timer;

        uint32_t width = 1366;
        uint32_t height = 768;

        void render(RenderInfo& renderInfo)
        {
            BarrierDesc bFragCopy[] = { {
                .srcAccess = PipelineAccess::Write,
                .srcStage = PipelineStage::ColorAttachmentOutput,
                .dstAccess = PipelineAccess::Read,
                .dstStage = PipelineStage::Copy,
            }, };

            BarrierDesc bColorOutputVert[] = { {
                .srcAccess = PipelineAccess::Write,
                .srcStage = PipelineStage::ColorAttachmentOutput,
                .dstAccess = PipelineAccess::Read,
                .dstStage = PipelineStage::VertexShader,
            }, };

            viewport->Record(renderInfo, *workspace);

            renderInfo.cmd.Barrier(bColorOutputVert);

            uiOverlay->Record(renderInfo);

            if (input.GetState().mouseLeftPressed)
            {
                renderInfo.cmd.Barrier(bFragCopy);
                viewport->RequestPick(renderInfo.cmd, input.GetState().mousePosition.x, input.GetState().mousePosition.y);
            }
        }

        void mainLoop()
        {
            timer.Start();

            double frameTime = 0.0;
            double fps = 0.0;

            double fpsAccumulator = 0.0;
            uint32_t frameCount = 0;

            uint32_t oldFbWidth = width;
            uint32_t oldFbHeight = height;
            while (!window->ShouldClose())
            {
                timer.Tick();
                input.Update(window->GetHandle());
                auto [wwidth, wheight] = window->GetSize();
                uiOverlay->Update(wwidth, wheight, timer, input.GetState());
                workspace->Update(timer, input.GetState(), viewport->GetPickResult());

                if (width == 0 || height == 0)
                {
                    window->WaitEvents();
                    continue;
                }

                auto& device = renderer->GetDevice();
                auto swapchain = renderer->GetSwapchain();
                auto cmdAlloc = renderer->GetCommandAllocator();

                auto fbSize = device.NextFrame(swapchain);

                if (fbSize.width != oldFbWidth || fbSize.height != oldFbHeight) [[unlikely]]
                {
                    viewport->Resize(fbSize.width, fbSize.height);
                    renderer->Resize(fbSize.width, fbSize.height);

                    oldFbWidth = fbSize.width;
                    oldFbHeight = fbSize.height;
                }

                device.Reset(cmdAlloc);
                auto frame = device.GetCurrentRenderTarget(swapchain);
                auto cmd = device.AllocateCommandBuffer(cmdAlloc);
                auto renderInfo = RenderInfo{ cmd, frame, renderer->GetDepthTarget(), viewport->GetPickTarget(), fbSize.width, fbSize.height };

                render(renderInfo);

                std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

                CommandBufferSubmitDesc submitDesc = {
                    .queueType = QueueType::Graphics,
                    .commandBuffers = std::span(cmds),
                    .presentSwapchain = swapchain,
                };

                device.Submit(submitDesc);

                device.DisplayFrame(swapchain);
                device.WaitFor(QueueType::Graphics);

                frameTime = timer.GetDeltaTime();
                fpsAccumulator += frameTime;
                ++frameCount;

                if (fpsAccumulator >= 1.0)
                {
                    fps = (double)(frameCount) / fpsAccumulator;
                    auto avg = 1000.0 * fpsAccumulator / (double)frameCount;

                    std::println("FPS {:4.0f} | avg {:.2f} ms", fps, avg);

                    fpsAccumulator = 0;
                    frameCount = 0;
                }

                window->PollEvents();
            }

            renderer->GetDevice().WaitFor(QueueType::Graphics);
        }

    public:
        void Run()
        {
            window = std::make_unique<Window>(width, height, "Lettuce Editor");
            renderer = std::make_unique<Renderer>(*window);
            uiOverlay = std::make_unique<UIOverlay>(renderer->GetDevice(), renderer->GetSwapchain());
            workspace = std::make_unique<SceneWorkspace>(renderer->GetDevice());
            viewport = std::make_unique<EditorViewport>(*window, renderer->GetDevice(), renderer->GetSwapchain());
            mainLoop();
            viewport.reset();
            workspace.reset();
            uiOverlay.reset();
            renderer.reset();
            window.reset();
        }
    };
};

int main()
{
    std::ios::sync_with_stdio(true);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    auto app = Editor::App();
    app.Run();
    return 0;
}