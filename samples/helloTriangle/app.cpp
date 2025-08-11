#include "Lettuce.Core.hpp"

#include <memory>

std::shared_ptr<Device> device;
std::shared_ptr<Swapchain> swapchain;
std::shared_ptr<Memory> meshMemory;
std::shared_ptr<Buffer> meshBuffer; // vertices | indices
std::shared_ptr<Memory> hostMemory;
std::shared_ptr<Buffer> ubo;
std::shared_ptr<DescriptorTable> descriptorTable;
std::shared_ptr<PipelineLayout> playout;
std::shared_ptr<Pipeline> pipeline;
std::shared_ptr<RenderTarget> colorTarget;

void initLettuce()
{
    device = std::make_shared<Device>();
    
    SwapchainCreateInfo swapchainCI = {


    };
    swapchain = device->CreateObject(swapchainCI);

    DescriptorTableCreateInfo descriptorTableCI = {

    };
    descriptorTable = device->CreateObject(descriptorTableCI);

    PipelineLayoutCreateInfo playoutCI = { 

    };
    playout = device->CreateObject(playoutCI);

    GraphicsPipelineCreateInfo pipelineCI = {

    };
    pipeline = device->CreateObject(pipelineCI);

    RenderTargetCreateInfo colorTargetCI = {
        .width = swapchain->GetWidth(),
        .height = swapchain->GetHeight(),
        .depth = 1,
        .format = swapchain->GetFormat(),
        .components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
    };
    colorTarget = device->CreateObject(colorTargetCI);

    meshMemory = device->MemoryAlloc();

    meshBuffer = device->CreateObject(BufferCreateInfo{
        .size = 1024 * 1024, // 1 MiB
        .usage = BufferUsage::VertexBuffer | BufferUsage::IndexBuffer,
        .memory = meshMemory,
    });

    hostMemory = device->MemoryAlloc(1024 * 1024, MemoryAccess::FastCPUWrite);

    ubo = device->CreateObject(BufferCreateInfo{
        .size = 256, // 256 bytes for UBO
        .usage = BufferUsage::UniformBuffer,
        .memory = hostMemory,
    });
}

void mainLoop()
{
    /*
    someObject->Record();
    someObject->Execute();
    someObject->Wait();
    someObject->Present(swapchain);
    someObject->Wait();
    */
}

void cleanupLettuce()
{

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