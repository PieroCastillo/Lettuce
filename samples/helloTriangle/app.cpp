#include "Lettuce/Lettuce.Core.hpp"

#include <memory>
#include <vector>

using namespace Lettuce::Core;

std::shared_ptr<Device> device;
std::shared_ptr<Swapchain> swapchain;
// std::shared_ptr<Memory> meshMemory;
// std::shared_ptr<Buffer> meshBuffer; // vertices | indices
// std::shared_ptr<Memory> hostMemory;
// std::shared_ptr<Buffer> ubo;
// std::shared_ptr<DescriptorTable> descriptorTable;
// std::shared_ptr<PipelineLayout> playout;
// std::shared_ptr<Pipeline> pipeline;
std::shared_ptr<RenderTarget> colorTarget;
std::shared_ptr<DeviceExecutionContext> executionContext;

void initLettuce()
{
    device = std::make_shared<Device>();

    SwapchainCreateInfo swapchainCI = {

    };
    swapchain = device->CreateObject(swapchainCI);

    // DescriptorTableCreateInfo descriptorTableCI = {

    // };
    // descriptorTable = device->CreateObject(descriptorTableCI);

    // PipelineLayoutCreateInfo playoutCI = {

    // };
    // playout = device->CreateObject(playoutCI);

    // GraphicsPipelineCreateInfo pipelineCI = {

    // };
    // pipeline = device->CreateObject(pipelineCI);
    
    RenderTargetCreateInfo colorTargetCI = {
            .width = swapchain->GetWidth(),
            .height = swapchain->GetHeight(),
            .depth = 1,
            .format = swapchain->GetFormat(),
            .components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    }
    colorTarget = device->CreateObject(colorTargetCI);

    // meshMemory = device->MemoryAlloc();

    // meshBuffer = device->CreateObject(BufferCreateInfo{
    //     .size = 1024 * 1024, // 1 MiB
    //     .usage = BufferUsage::VertexBuffer | BufferUsage::IndexBuffer,
    //     .memory = meshMemory,
    //     });

    // hostMemory = device->MemoryAlloc(1024 * 1024, MemoryAccess::FastCPUWrite);

    // ubo = device->CreateObject(BufferCreateInfo{
    //     .size = 256, // 256 bytes for UBO
    //     .usage = BufferUsage::UniformBuffer,
    //     .memory = hostMemory,
    //     });

    executionContext = device->CreateObject(DeviceExecutionContextCreateInfo{
        .threadCount = 4,
        .maxTasks = 4,
        });

    CommandsList cmdList = {
        .type = CommandListType::Graphics,
    };
    cmdList.Record([&](CommandRecordingContext& ctx)
        {
            ctx.BindPipeline(pipeline);
            ctx.BindDescriptorTable(descriptorTable);
            ctx.BindVertexBuffers(meshBuffer);
            ctx.BindIndexBuffer(meshBuffer);
            ctx.DrawIndexed(3, 1, 0, 0, 0); });

    std::vector<std::vector<CommandList>> cmds;
    cmd.push_back({ cmdList });

    executionContext->Prepare(cmds);
}

void mainLoop()
{
    swapchain->AcquireNextImage();
    executionContext->Record();
    executionContext->Execute();
    swapchain->Present();
}

void cleanupLettuce()
{
    executionContext->Release();
    colorTarget->Release();
    // ubo->Release();
    // hostMemory->Release();
    // meshBuffer->Release();
    // meshMemory->Release();
    pipeline->Release();
    playout->Release();
    descriptorTable->Release();
    swapchain->Release();
    device->Release();
}

void initWindow()
{
}

void cleanupWindow()
{
}

int main()
{
    initWindow();
    initLettuce();
    mainLoop();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}