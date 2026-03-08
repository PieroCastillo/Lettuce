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

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include <meshoptimizer.h>

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;

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
    glm::vec3 position = { 0.0f, 0.0f, 3.0f };
    glm::quat orientation = { 1.0f, 0.0f, 0.0f, 0.0f };
};

struct SceneData
{
    float4x4 viewProj;
};

struct Instance {
    uint32_t meshID;
    float4x4 model;
};

struct MeshInfo {
    uint32_t primitiveOffset;
    uint32_t primitiveCount;
};

struct PrimitiveInfo {
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t indexOffset;
    uint32_t indexCount;
};

struct Vertex {
    float3 position;
    float3 normal;
    float3 tangent;
    float2 texCoord0;
};

struct InstancedPrimitive
{
    uint32_t primitiveID, instanceID;
};

GLFWwindow* window;

constexpr uint32_t width = 1366;
constexpr uint32_t height = 768;

Device device;
Swapchain swapchain;
DescriptorTable descriptorTable;
Pipeline cullPipeline;
Pipeline rgbPipeline;
CommandAllocator cmdAlloc;

Allocators::LinearAllocator memalloc;
MemoryView mvSceneData;
MemoryView mvIndexB, mvVertexB, mvInstances, mvMeshes, mvPrimitives, mvInstancedPrimitives;
MemoryView mvIndirectB;
MemoryView mvDebugBuffer;
IndirectSet isIndirect;

std::vector<MeshInfo> meshes;
std::vector<PrimitiveInfo> primitives;

constexpr uint32_t debugBufferCount = 32;
constexpr uint32_t debugBufferItemSize = 4 * sizeof(uint32_t);

FrameTimer timer;
CameraState camera;

inline uint8_t firstByte(const MemoryView& mv)
{
    return *(uint8_t*)mv.cpuAddress;
}

inline uint64_t firstUInt64(const MemoryView& mv)
{
    return *(uint64_t*)mv.cpuAddress;
}

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
bool wasMousePressed = false;

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

    if (mousePressed && !wasMousePressed)
    {
        xprev = xpos;
        yprev = ypos;
    }

    ((SceneData*)(mvSceneData.cpuAddress))[0].viewProj = calculateViewProjection(camera, xpos, ypos, xprev, yprev,
        mousePressed, wKeyPressed, aKeyPressed, sKeyPressed, dKeyPressed,
        dt, 60.0f, width / float(height), 0.1f, 100.0f);

    xprev = xpos;
    yprev = ypos;
    wasMousePressed = mousePressed;
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
        .bufferSize = 100 * 1024 * 1024, // 100 MB
        .imageSize = 16,
        .cpuVisible = true,
    };
    memalloc.Create(device, lindesc);
    mvSceneData = memalloc.AllocateMemory(sizeof(SceneData));

    IndirectSetDesc isDesc =
    {
        .type = IndirectType::Draw,
        .maxCount = 1024,
        .userDataSize = 0,
    };
    isIndirect = device.CreateIndirectSet(isDesc);
    mvIndirectB = device.GetIndirectSetView(isIndirect);

    // DEBUG BUFFER, CPU READEABLE
    mvDebugBuffer = memalloc.AllocateMemory(debugBufferCount * debugBufferItemSize);
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

    ComputePipelineDesc compPipelineDesc = {
        .compEntryPoint = "cullMain",
        .compShaderBinary = shaders,
        .descriptorTable = descriptorTable,
    };
    cullPipeline = device.CreatePipeline(compPipelineDesc);

    device.Destroy(shaders);
}

void loadModel()
{
    std::filesystem::path modelPath = "../../../../external/models/DragonAttenuation.glb";

    if (!std::filesystem::exists(modelPath))
    {
        std::println("file {} does not exist", "DragonAttenuation.glb");
        return;
    }

    auto parser = fastgltf::Parser();
    auto gltfData = fastgltf::GltfDataBuffer::FromPath(modelPath);

    auto asset = parser.loadGltf(gltfData.get(), modelPath.parent_path(), fastgltf::Options::None);

    meshes = std::vector<MeshInfo>();
    primitives = std::vector<PrimitiveInfo>();
    auto vertexVec = std::vector<Vertex>();
    auto indexVec = std::vector<uint32_t>();

    int meshCount = 0;
    int totalPrims = 0;
    for (auto& mesh : asset->meshes)
    {
        int primCount = 0;
        for (auto& prim : mesh.primitives)
        {
            auto* posIt = prim.findAttribute("POSITION");
            auto* norIt = prim.findAttribute("NORMAL");
            auto* tanIt = prim.findAttribute("TANGENT");
            auto* uvsIt = prim.findAttribute("TEXCOORD_0");

            auto& posAcc = asset->accessors[posIt->accessorIndex];
            auto& norAcc = asset->accessors[norIt->accessorIndex];
            auto& tanAcc = asset->accessors[tanIt->accessorIndex];
            auto& uvsAcc = asset->accessors[uvsIt->accessorIndex];
            auto& idxAcc = asset->accessors[prim.indicesAccessor.value()];

            auto baseVertexIdx = vertexVec.size();
            auto baseIndexIdx = indexVec.size();

            vertexVec.resize(baseVertexIdx + posAcc.count);
            indexVec.resize(baseIndexIdx + idxAcc.count);

            PrimitiveInfo prim = { baseVertexIdx, posAcc.count, baseIndexIdx, idxAcc.count };
            primitives.push_back(prim);

            fastgltf::iterateAccessorWithIndex<float3>(asset.get(), posAcc, [&](float3 pos, size_t idx) {
                vertexVec[baseVertexIdx + idx].position = pos;
                });
            std::println("pos acc");
            fastgltf::iterateAccessorWithIndex<float3>(asset.get(), norAcc, [&](float3 normal, size_t idx) {
                vertexVec[baseVertexIdx + idx].normal = normal;
                });
            std::println("nor acc");

            fastgltf::iterateAccessorWithIndex<float3>(asset.get(), tanAcc, [&](float3 tang, size_t idx) {
                vertexVec[baseVertexIdx + idx].tangent = tang;
                });
            std::println("tan acc");

            fastgltf::iterateAccessorWithIndex<float2>(asset.get(), uvsAcc, [&](float2 uv, size_t idx) {
                vertexVec[baseVertexIdx + idx].texCoord0 = uv;
                });
            std::println("tex0 acc");

            fastgltf::iterateAccessorWithIndex<uint32_t>(asset.get(), idxAcc, [&](uint32_t index, size_t idx) {
                indexVec[baseIndexIdx + idx] = index;
                });
            std::println("idx acc");

            std::println("mesh #{}, primitive  #{} : #vertex:{}  #index: {}", meshCount, primCount, posAcc.count, idxAcc.count);
            ++primCount;
        }
        MeshInfo mesh;
        mesh.primitiveOffset = totalPrims;
        mesh.primitiveCount = primCount;
        meshes.push_back(mesh);
        totalPrims += primCount;
        ++meshCount;
    }

    mvMeshes = memalloc.AllocateMemory(sizeof(MeshInfo) * meshes.size());
    mvPrimitives = memalloc.AllocateMemory(sizeof(PrimitiveInfo) * primitives.size());
    mvVertexB = memalloc.AllocateMemory(sizeof(Vertex) * vertexVec.size());
    mvIndexB = memalloc.AllocateMemory(sizeof(uint32_t) * indexVec.size());

    memcpy(mvMeshes.cpuAddress, meshes.data(), sizeof(MeshInfo) * meshes.size());
    memcpy(mvPrimitives.cpuAddress, primitives.data(), sizeof(PrimitiveInfo) * primitives.size());
    memcpy(mvVertexB.cpuAddress, vertexVec.data(), sizeof(Vertex) * vertexVec.size());
    memcpy(mvIndexB.cpuAddress, indexVec.data(), sizeof(uint32_t) * indexVec.size());
}

uint32_t instanceCount = 0;
uint32_t instancedPrimitivesCount = 0;
void loadInstances()
{
    std::vector<Instance> instances;
    instances.reserve(20);

    const int gridX = 5;
    const int gridY = 4;
    const float spacing = 8.0f;

    for (int y = 0; y < gridY; y++)
    {
        for (int x = 0; x < gridX; x++)
        {
            int i = y * gridX + x;

            glm::vec3 pos = {
                (x - gridX / 2) * spacing,
                0.0f,
                (y - gridY / 2) * spacing
            };

            float angle = glm::radians(20.0f * i);

            float uniformScale = 0.5f;
            glm::mat4 model =
                glm::translate(glm::mat4(1.0f), pos) *
                glm::rotate(glm::mat4(1.0f), angle, { 0,1,0 }) *
                glm::scale(glm::mat4(1.0f), { uniformScale,uniformScale,uniformScale });

            Instance inst;
            inst.meshID = (i < 10) ? 0 : 1;
            inst.model = model;

            instancedPrimitivesCount += meshes[inst.meshID].primitiveCount;

            instances.push_back(inst);
        }
    }

    mvInstances = memalloc.AllocateMemory(sizeof(uint32_t) + (sizeof(Instance) * instances.size()));
    mvInstancedPrimitives = memalloc.AllocateMemory(sizeof(InstancedPrimitive) * instancedPrimitivesCount);
    // instances Buffer layout: [ count | instances ]
    instanceCount = instances.size();
    *(uint32_t*)(mvInstances.cpuAddress) = instances.size();
    memcpy(sizeof(uint32_t) + (uint8_t*)(mvInstances.cpuAddress), instances.data(), sizeof(Instance) * instances.size());
}

void mainLoop()
{
    timer.Start();

    while (!glfwWindowShouldClose(window))
    {
        timer.Tick();
        UpdateCamera();

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
                std::pair(1u, mvInstances),
                std::pair(2u, mvMeshes),
                std::pair(3u, mvPrimitives),
                std::pair(4u, mvVertexB),
                std::pair(5u, mvIndexB),
                std::pair(6u, mvIndirectB),
                std::pair(7u, mvInstancedPrimitives),
                std::pair(8u, mvDebugBuffer),
            },
            .descriptorTable = descriptorTable,
        };

        ExecuteIndirectDesc execDesc = {
            .indirectSet = isIndirect,
            .maxDrawCount = instancedPrimitivesCount,
        };

        BarrierDesc bCopyComp[] = { {
            .srcAccess = PipelineAccess::Write,
            .srcStage = PipelineStage::ComputeShader,
            .dstAccess = PipelineAccess::Read,
            .dstStage = PipelineStage::ComputeShader,
        }, };

        BarrierDesc bCompInd[] = { {
            .srcAccess = PipelineAccess::Write,
            .srcStage = PipelineStage::ComputeShader,
            .dstAccess = PipelineAccess::Read,
            .dstStage = PipelineStage::DrawIndirect,
        }, };

        cmd.ResetCount(isIndirect);

        cmd.Barrier(bCopyComp);

        cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Compute);
        cmd.BindPipeline(cullPipeline);
        cmd.PushAllocations(pushDesc);
        auto dispatchX = (instanceCount + 31) / 32;
        cmd.Dispatch(dispatchX, 1, 1);

        cmd.Barrier(bCompInd);

        cmd.BeginRendering(renderPassDesc);
        cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Graphics);
        cmd.BindPipeline(rgbPipeline);
        cmd.PushAllocations(pushDesc);
        cmd.ExecuteIndirect(execDesc);
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

        // auto baseGenDrawCallPtr = (VkDrawIndirectCommand*)mvDebugBuffer.cpuAddress;
        // for (size_t i = 0; i < debugBufferCount; ++i) {
        //     auto genDrawCallPtr = (baseGenDrawCallPtr + i);
        //     std::print(" {},{},{},{} |", genDrawCallPtr->vertexCount, genDrawCallPtr->instanceCount, genDrawCallPtr->firstVertex, genDrawCallPtr->firstVertex);
        // }
        // std::println();

        glfwPollEvents();
    }
}

void cleanupLettuce()
{
    device.WaitFor(QueueType::Graphics);
    device.Destroy(rgbPipeline);
    device.Destroy(cullPipeline);
    device.Destroy(descriptorTable);

    memalloc.Destroy();
    device.Destroy(isIndirect);
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
    loadModel();
    loadInstances();
    mainLoop();
    cleanupLettuce();
    cleanupWindow();
    return 0;
}