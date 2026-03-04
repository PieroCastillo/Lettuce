#include "Lettuce/Lettuce.hpp"
#include "glfw/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <windows.h>

#include <memory>
#include <vector>
#include <expected>
#include <thread>
#include <chrono>
#include <print>
#include <fstream>
#include <filesystem>
#include <source_location>
#include <optional>
#include <functional>

using namespace Lettuce::Core;

class FrameTimer
{
public:
    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;
    using seconds_f = std::chrono::duration<float>;

    void Start()
    {
        m_start = clock::now();
        m_last = m_start;
        m_delta = 0.0f;
        m_running = true;
    }

    void Tick() // marcar fin de frame y preparar siguiente
    {
        if (!m_running) return;

        time_point now = clock::now();
        m_delta = std::chrono::duration_cast<seconds_f>(now - m_last).count();
        m_last = now;
    }

    float GetDeltaTime() const
    {
        return m_delta;
    }

    float GetTotalTime() const
    {
        if (!m_running) return 0.0f;
        return std::chrono::duration_cast<seconds_f>(clock::now() - m_start).count();
    }

private:
    time_point m_start{};
    time_point m_last{};
    float m_delta{ 0.0f };
    bool m_running{ false };
};

struct CameraState
{
    glm::vec3 position = { 0.0f, 0.0f, 5.0f };
    glm::quat orientation = { 1.0f, 0.0f, 0.0f, 0.0f };
};

struct SceneData
{
    glm::mat4 viewProj;
};

GLFWwindow* window;

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

Device device;
Swapchain swapchain;
DescriptorTable descriptorTable;
Pipeline rgbPipeline;
CommandAllocator cmdAlloc;

Allocators::LinearAllocator memalloc;
MemoryView mvSceneData;

FrameTimer timer;
CameraState camera;

glm::mat4 calculateViewProjection(
    CameraState& cam,
    uint32_t currentX,
    uint32_t currentY,
    uint32_t prevX,
    uint32_t prevY,
    bool leftClickPressed,
    bool keyW,
    bool keyA,
    bool keyS,
    bool keyD,
    float deltaTime,
    float fovY,
    float aspect,
    float nearPlane,
    float farPlane)
{
    if (leftClickPressed)
    {
        float sensitivity = 0.0025f;

        float dx = static_cast<float>(currentX) - static_cast<float>(prevX);
        float dy = static_cast<float>(currentY) - static_cast<float>(prevY);

        glm::quat yaw = glm::angleAxis(-dx * sensitivity, glm::vec3(0, 1, 0));
        glm::vec3 right = cam.orientation * glm::vec3(1, 0, 0);
        glm::quat pitch = glm::angleAxis(-dy * sensitivity, right);

        cam.orientation = glm::normalize(pitch * yaw * cam.orientation);
    }

    float speed = 5.0f * deltaTime;
    glm::vec3 forward = cam.orientation * glm::vec3(0, 0, -1);
    glm::vec3 right = cam.orientation * glm::vec3(1, 0, 0);

    if (keyW) cam.position += forward * speed;
    if (keyS) cam.position -= forward * speed;
    if (keyA) cam.position -= right * speed;
    if (keyD) cam.position += right * speed;

    glm::mat4 rotation = glm::toMat4(glm::conjugate(cam.orientation));
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), -cam.position);
    glm::mat4 view = rotation * translation;

    glm::mat4 projection = glm::perspective(
        glm::radians(fovY),
        aspect,
        nearPlane,
        farPlane
    );

    projection[1][1] *= -1.0f;

    return projection * view;
}

double xprev = width / 2;
double yprev = height / 2;

void UpdateCamera()
{
    double dt = timer.GetDeltaTime();
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    auto mousePressed = GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    auto aKeyPressed = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_LEFT);
    auto wKeyPressed = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_UP);
    auto sKeyPressed = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_DOWN);
    auto dKeyPressed = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_RIGHT);

    ((SceneData*)(mvSceneData.cpuAddress))->viewProj = calculateViewProjection(camera, xpos, ypos, xprev, yprev,
        mousePressed, wKeyPressed, aKeyPressed, sKeyPressed, dKeyPressed,
        dt, 60.0f, width / float(height), 0.1f, 100.0f);

    xprev = xpos;
    xprev = ypos;
}

std::vector<uint32_t> loadSpv(std::string path)
{
    auto shadersFile = std::ifstream(path, std::ios::ate | std::ios::binary);
    if (!shadersFile) throw std::runtime_error(path + " does not exist");

    auto fileSize = (uint32_t)shadersFile.tellg();
    std::vector<uint32_t> shadersBuffer;
    shadersBuffer.resize(fileSize / sizeof(uint32_t));

    shadersFile.seekg(0);
    shadersFile.read((char*)shadersBuffer.data(), fileSize);

    return shadersBuffer;
}

void initLettuce()
{
    auto hwnd = glfwGetWin32Window(window);
    auto hmodule = GetModuleHandle(NULL);

    DeviceDesc deviceCI = {
        .preferDedicated = true,
    };
    device.Create(deviceCI);

    SwapchainDesc swapchainDesc = {
        .width = width,
        .height = height,
        .clipped = true,
        .windowPtr = &hwnd,
        .applicationPtr = &hmodule,
    };
    swapchain = device.CreateSwapchain(swapchainDesc);

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Graphics,
    };
    cmdAlloc = device.CreateCommandAllocator(cmdAllocDesc);

    Allocators::LinearAllocatorDesc lindesc =
    {
        .bufferSize = 1024 * 1024, // 1 MB
        .imageSize = 16,
        .cpuVisible = true,
    };
    memalloc.Create(device, lindesc);
    mvSceneData = memalloc.AllocateMemory(sizeof(SceneData));
}

void createRenderingObjects()
{
    auto shadersBuffer = loadSpv("loadModel.spv");

    ShaderBinaryDesc shaderDesc = {
        .bytecode = std::span<uint32_t>(shadersBuffer.data(), shadersBuffer.size()),
    };
    auto shaders = device.CreateShader(shaderDesc);

    DescriptorTableDesc descriptorTableDesc = { 4,4,4 };
    descriptorTable = device.CreateDescriptorTable(descriptorTableDesc);

    std::array<Format, 1> formatArr = { device.GetRenderTargetFormat(swapchain) };
    PrimitiveShadingPipelineDesc pipelineDesc = {
        .fragmentShadingRate = false,
        .vertEntryPoint = "vertexMain",
        .fragEntryPoint = "fragmentMain",
        .vertShaderBinary = shaders,
        .fragShaderBinary = shaders,
        .colorAttachmentFormats = std::span(formatArr),
        .descriptorTable = descriptorTable,
    };
    rgbPipeline = device.CreatePipeline(pipelineDesc);

    device.Destroy(shaders);
}

void mainLoop()
{
    timer.Start();

    while (!glfwWindowShouldClose(window))
    {
        timer.Tick();
        UpdateCamera();
        // std::println("quat -> {} + {} i + {} j + {} k", camera.orientation.w, camera.orientation.x, camera.orientation.y, camera.orientation.z);
        // std::println("pos: ({},{},{})", camera.position.x, camera.position.y, camera.position.z);
        // std::println("delta time: {}", timer.GetDeltaTime());

        device.NextFrame(swapchain);

        device.Reset(cmdAlloc);
        auto frame = device.GetCurrentRenderTarget(swapchain);
        auto cmd = device.AllocateCommandBuffer(cmdAlloc);

        AttachmentDesc colorAttachment[1] = {
            {
                .renderTarget = frame,
                .loadOp = LoadOp::Clear,
            }
        };

        RenderPassDesc renderPassDesc = {
            .width = width,
            .height = height,
            .colorAttachments = std::span(colorAttachment),
        };

        PushAllocationsDesc pushDesc = {
             .allocations = std::array {
                std::pair(0u, mvSceneData),
            },
            .descriptorTable = descriptorTable,
        };

        cmd.BeginRendering(renderPassDesc);
        cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Graphics);
        cmd.BindPipeline(rgbPipeline);
        cmd.PushAllocations(pushDesc);
        cmd.Draw(3, 1);
        cmd.EndRendering();

        std::array<std::span<CommandBuffer>, 1> cmds = { std::span(&cmd, 1) };

        CommandBufferSubmitDesc submitDesc = {
            .queueType = QueueType::Graphics,
            .commandBuffers = std::span(cmds),
            .presentSwapchain = swapchain,
        };
        device.Submit(submitDesc);

        device.DisplayFrame(swapchain);
        device.WaitFor(QueueType::Graphics);
        glfwPollEvents();   
    }
}

void cleanupLettuce()
{
    device.WaitFor(QueueType::Graphics);
    device.Destroy(rgbPipeline);
    device.Destroy(descriptorTable);

    memalloc.Destroy();
    device.Destroy(cmdAlloc);
    device.Destroy(swapchain);
    device.Destroy();
}

void initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "My Lettuce Window", NULL, NULL);
}

void cleanupWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main()
{
    std::ios::sync_with_stdio(true);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    initWindow();
    initLettuce();
    createRenderingObjects();
    mainLoop();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}