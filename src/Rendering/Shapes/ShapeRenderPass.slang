module LettuceRendering;

__include "../shaderShared/std.part.slang";

Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

struct PushConstants
{
    uint64_t sceneViewDataAddr;
    uint64_t materialShadingDataAddr;
    uint64_t materialStorageAddr;
    uint64_t indirectCommandsStorage;

    uint64_t sphereStorageAddr;
    uint64_t prismStorageAddr;
    uint64_t cylinderStorageAddr;
};

[[vk::push_constant]]
PushConstants pc;

[shader("vertex")]
std.DeferredVSOutput vertMain(uint vertexID: SV_VulkanVertexID, uint instanceID: SV_DrawIndex)
{
    std.DeferredVSOutput output;

    return output;
}

[shader("fragment")]
std.DeferredPSOutput fragMain(std.DeferredVSOutput input)
{
    return std.deferredFragBase(input);
}