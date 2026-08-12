// standard headers
#include <memory>
#include <print>
#include <vector>

// project headers
#include "Lettuce/Lettuce.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "Input.hpp"
#include "SceneEditor.hpp"
#include "UIOverlay.hpp"

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

        std::unique_ptr<SceneEditor> sceneEditor;
        std::unique_ptr<UIOverlay> uiOverlay;

        DescriptorTable descriptorTable;
        std::unique_ptr<Debug::DebugPass> debugPass;

        GpuUniquePtr<uint32_t> pickInstance;

        Lettuce::Utils::FrameTimer timer;

        uint32_t width = 1366;
        uint32_t height = 768;
        uint32_t oldFbWidth = width;
        uint32_t oldFbHeight = height;

        void createRenderingObjects()
        {
            auto& device = renderer->GetDevice();
            auto swapchain = renderer->GetSwapchain();

            // load buffers
            pickInstance = GpuUniquePtr<uint32_t>(device);

            DescriptorTableDesc descriptorTableDesc = { 4,4,4 };
            descriptorTable = device.CreateDescriptorTable(descriptorTableDesc);

            Debug::DebugPassDesc debugPassDesc = {
                .device = device,
                .descriptorTable = descriptorTable,
                .maxCulledInstances = 10,
                .colorOutputFormat = device.GetRenderTargetFormat(swapchain),
            };
            debugPass = std::make_unique<Debug::DebugPass>(debugPassDesc);
        }

        void mainLoop()
        {
            timer.Start();

            while (!window->ShouldClose())
            {
                timer.Tick();
                input.Update(window->GetHandle());
                sceneEditor->Update(timer, input);

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
                    renderer->Resize(fbSize.width, fbSize.height);

                device.Reset(cmdAlloc);
                auto frame = device.GetCurrentRenderTarget(swapchain);
                auto cmd = device.AllocateCommandBuffer(cmdAlloc);

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

                Debug::DebugPassRecordDesc record = {
                    .fbWidth = fbSize.width,
                    .fbHeight = fbSize.height,
                    .sceneViewData = sceneEditor->GetViewData(),
                    .positions = sceneEditor->GetView().GetPositionsView(),
                    .indices = sceneEditor->GetView().GetIndicesView(),
                    .clusters = sceneEditor->GetView().GetClustersView(),
                    .meshes = sceneEditor->GetView().GetMeshesView(),
                    .culledInstances = sceneEditor->GetView().GetInstanceTable(),
                    .rtColorOutput = frame,
                    .rtDepth = renderer->GetDepthTarget(),
                    .rtPick = renderer->GetPickTarget(),
                };
                debugPass->Record(cmd, record);

                cmd.Barrier(bColorOutputVert);

                auto renderInfo = RenderInfo{ cmd, frame, renderer->GetDepthTarget(), renderer->GetPickTarget(), fbSize.width, fbSize.height };

                uiOverlay->Record(renderInfo);

                if (input.GetState().mouseLeftPressed)
                {
                    cmd.Barrier(bFragCopy);
                    Lettuce::Utils::Algorithm::LazyCopyPixel<uint32_t>(cmd, renderer->GetPickTarget(), input.GetState().mousePosition.x, input.GetState().mousePosition.y, pickInstance);
                }
                else
                {
                    *pickInstance = 0;
                }
                std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

                CommandBufferSubmitDesc submitDesc = {
                    .queueType = QueueType::Graphics,
                    .commandBuffers = std::span(cmds),
                    .presentSwapchain = swapchain,
                };

                device.Submit(submitDesc);

                device.DisplayFrame(swapchain);
                device.WaitFor(QueueType::Graphics);

                if (input.GetState().mouseLeftPressed)
                {
                    uint32_t value = *pickInstance;
                    if (value == 0)
                        std::println("no instance selected");
                    else if ((value & (1 << 31)) != 0)
                        std::println("instance 2D selected: {}", (value & ((1U << 31) - 1)) - 1);
                    else
                        std::println("instance 3D selected: {}", value - 1);
                }

                // first bit is 0 at 3D instances
                // if (isPressed && (*pickInstance & (1 << 31)) == 0)
                // {

                // }

                window->PollEvents();
            }
        }

        void cleanupRenderingObjects()
        {
            auto& device = renderer->GetDevice();
            device.WaitFor(QueueType::Graphics);

            debugPass.reset();
            device.Destroy(descriptorTable);
            pickInstance.reset();
        }

    public:
        void Run()
        {
            window = std::make_unique<Window>(width, height, "Lettuce Editor");
            renderer = std::make_unique<Renderer>(*window);
            uiOverlay = std::make_unique<UIOverlay>(renderer->GetDevice(), renderer->GetSwapchain());
            sceneEditor = std::make_unique<SceneEditor>(renderer->GetDevice());
            createRenderingObjects();
            mainLoop();
            cleanupRenderingObjects();
            sceneEditor.reset();
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