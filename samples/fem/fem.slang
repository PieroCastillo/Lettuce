Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

const static uint threadCount = 32;

struct SimulationInfo
{
    uint32_t elementCount;
};

struct Node
{
    uint position;
    uint displacement;
    uint force;
};

struct Element
{
    uint4 nodeIdxs;

    float volume;
    float B[6][12];
};

struct PushContants
{
    uint64_t simulationInfoAddr;
    uint64_t nodesAddr;
    uint64_t elementsAddr;
};
[[vk::push_constant]]
PushContants pc;

[require(compute, spirv_1_6)]
[numthreads(threadCount, 1, 1)]
void mainComp(uint gid : SV_DispatchThreadID)
{
    // var simulationInfo = reinterpret<Ptr<SimulationInfo>>(pc.simulationInfoAddr)[0];
    // var nodes = reinterpret<Ptr<Node>>(pc.nodesAddr);
    // var elements = reinterpret<Ptr<Element>>(pc.elementsAddr);

    // if (simulationInfo.elementCount <= gid.x)
    //     return;

    // var element = elements[gid.x];
    // for (uint i = 0; i < maxItex; ++i)
    // {

    // }
}