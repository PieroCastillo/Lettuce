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
        uint32_t oldFbWidth = width;
        uint32_t oldFbHeight = height;

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

            while (!window->ShouldClose())
            {
                timer.Tick();
                input.Update(window->GetHandle());
                workspace->Update(timer, input, viewport->GetPickResult());

                auto& device = renderer->GetDevice();
                auto swapchain = renderer->GetSwapchain();
                auto cmdAlloc = renderer->GetCommandAllocator();

                auto windowSize = window->GetSize();
                width = windowSize.first;
                height = windowSize.second;

                if (width == 0 || height == 0)
                {
                    window->WaitEvents();
                    continue;
                }

                auto fbSize = device.NextFrame(swapchain);

                if (fbSize.width != oldFbWidth || fbSize.height != oldFbHeight) [[unlikely]]
                {
                    viewport->Resize(fbSize.width, fbSize.height);
                    renderer->Resize(fbSize.width, fbSize.height);
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