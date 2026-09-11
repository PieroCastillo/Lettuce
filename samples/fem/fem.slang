Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

const static uint threadCount = 32;

// shaders in/out
struct VSOutput
{
    float4 position : SV_Position;
};

struct PSOutput
{
    float4 colorOutput : SV_Target0;
};

// buffers

struct SceneViewData
{
    float4x4 viewProj;
    uint32_t instanceCount;
};

struct Material
{
    float young;   // E
    float poisson; // nu
    float density; // uniform
};

struct SimulationInfo
{
    uint32_t elementCount;
    uint32_t nodeCount;
    uint32_t iterationCount;
    Material material;
};

struct Node
{
    float3 position;
    float3 displacement;
    float3 force;
    uint constraints; // bits: [highest, ... | X | Y | Z , lowest]
};

struct Element
{
    uint4 nodeIdxs;

    float volume;
    float B[6][12];
};

struct PushContantsComp
{
    uint64_t sceneViewDataAddr;
    uint64_t simulationInfoAddr;
    uint64_t nodesAddr;
    uint64_t elementsAddr;
};
[[vk::push_constant]]
PushContantsComp pcc;

[require(compute, spirv_1_6)]
[numthreads(threadCount, 1, 1)]
void compMain(uint gid: SV_DispatchThreadID)
{
    var simulationInfo = reinterpret<Ptr<SimulationInfo>>(pcc.simulationInfoAddr)[0];
    var nodes = reinterpret<Ptr<Node>>(pcc.nodesAddr);
    var elements = reinterpret<Ptr<Element>>(pcc.elementsAddr);

    if (simulationInfo.elementCount <= gid.x)
        return;

    var E = simulationInfo.material.young;
    var nu = simulationInfo.material.poisson;
    var mu = 0.5 * E * rcp(1 + nu);
    var lambda = 0.5 * E * nu * rcp(1 + nu) * rcp(1 - (2 * nu));

    // element per thread
    var element = elements[gid.x];
    var mass = 0.25f * simulationInfo.material.density * element.volume;
    float relaxationFactor = 0.01f;

    for (uint i = 0; i < simulationInfo.iterationCount; ++i)
    {
        // assembly
        float u[12];
        for (uint j = 0; j < 4; ++j)
        {
            var disp = nodes[element.nodeIdxs[j]].displacement;
            u[j * 3 + 0] = disp.x;
            u[j * 3 + 1] = disp.y;
            u[j * 3 + 2] = disp.z;
        }

        // deformation
        float strain[6] = { 0, 0, 0, 0, 0, 0 };
        for (int row = 0; row < 6; ++row)
        {
            for (int col = 0; col < 12; ++col)
            {
                strain[row] += element.B[row][col] * u[col];
            }
        }

        // strain
        float traceStrain = strain[0] + strain[1] + strain[2];
        float stress[6];
        stress[0] = 2.0 * mu * strain[0] + lambda * traceStrain;
        stress[1] = 2.0 * mu * strain[1] + lambda * traceStrain;
        stress[2] = 2.0 * mu * strain[2] + lambda * traceStrain;
        stress[3] = mu * strain[3];
        stress[4] = mu * strain[4];
        stress[5] = mu * strain[5];

        // internal forces
        float f_int[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        for (int col = 0; col < 12; ++col)
        {
            for (int row = 0; row < 6; ++row)
            {
                f_int[col] += element.B[row][col] * stress[row];
            }
            f_int[col] *= element.volume;
        }

        // update
        for (int n = 0; n < 4; ++n) {
            uint nodeIdx = element.nodeIdxs[n];
            uint constraints = nodes[nodeIdx].constraints;

            float3 deltaU = float3(-f_int[n * 3 + 0], -f_int[n * 3 + 1], -f_int[n * 3 + 2]) * relaxationFactor * rcp(mass);

            if ((constraints & 1) != 0) deltaU.x = 0;
            if ((constraints & 2) != 0) deltaU.y = 0;
            if ((constraints & 4) != 0) deltaU.z = 0;

            InterlockedAdd(nodes[nodeIdx].displacement.x, deltaU.x);
            InterlockedAdd(nodes[nodeIdx].displacement.y, deltaU.y);
            InterlockedAdd(nodes[nodeIdx].displacement.z, deltaU.z);
        }
    }
}

struct PushContantsShade
{
    uint64_t sceneViewDataAddr;
    uint64_t simulationInfoAddr;
    uint64_t nodesAddr;
    uint64_t elementsAddr;
    uint64_t positionsAddr;
    uint64_t indicesAddr;
};
[[vk::push_constant]]
PushContantsShade pcs;

[shader("vertex")]
VSOutput vertMain(uint vertexID: SV_VulkanVertexID)
{
    var sceneViewData = reinterpret<Ptr<SceneViewData>>(pcs.sceneViewDataAddr)[0];
    var simulationInfo = reinterpret<Ptr<SimulationInfo>>(pcs.simulationInfoAddr)[0];
    var nodes = reinterpret<Ptr<Node>>(pcs.nodesAddr);

    var positions = reinterpret<Ptr<float3>>(pcs.positionsAddr);
    var indices = reinterpret<Ptr<uint32_t>>(pcs.indicesAddr);

    // var pos = nodes[vertexID].position;
    var pos = positions[indices[vertexID]];

    VSOutput output;
    output.position = mul(sceneViewData.viewProj, float4(pos, 1.0));

    return output;
}

[require(fragment, spirv_1_6)]
[shader("fragment")]
PSOutput fragMain(VSOutput input)
{
    PSOutput output;
    output.colorOutput = float4(0.1, 0.4, 0.6, 1);

    return output;
}