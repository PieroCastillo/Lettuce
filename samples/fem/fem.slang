Texture2D<float4> sampledTextures[];
SamplerState samplers[];
RWTexture2D<float4> storageTextures[];

const static uint threadCount = 32;

// shaders in/out
struct VSOutput
{
    float4 position : SV_Position;
    float3 color;
    float3 baryCoord;
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
    float vonMisesStress;
    float stressCount;
    uint constraints; // bits: [highest, ... | X | Y | Z , lowest]
};

struct Element
{
    uint4 nodeIdxs;

    float volume;
    float B[6][12];
};

float3 StressToColor(float stress, float minStress, float maxStress)
{
    float t = saturate(
        (stress - minStress) / max(maxStress - minStress, 1e-20f));

    // azul -> cyan -> verde -> amarillo -> rojo
    if (t < 0.25f)
        return lerp(
            float3(0, 0, 1),
            float3(0, 1, 1),
            t * 4.0f);

    if (t < 0.50f)
        return lerp(
            float3(0, 1, 1),
            float3(0, 1, 0),
            (t - 0.25f) * 4.0f);

    if (t < 0.75f)
        return lerp(
            float3(0, 1, 0),
            float3(1, 1, 0),
            (t - 0.50f) * 4.0f);

    return lerp(
        float3(1, 1, 0),
        float3(1, 0, 0),
        (t - 0.75f) * 4.0f);
}

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
    var relaxationFactor = 1e-7f;

    float stress[6];
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

        // cauchy strain tensor
        float traceStrain = strain[0] + strain[1] + strain[2];

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
        for (int n = 0; n < 4; ++n)
        {
            uint nodeIdx = element.nodeIdxs[n];
            uint constraints = nodes[nodeIdx].constraints;

            float3 f_internal = float3(f_int[n * 3 + 0], f_int[n * 3 + 1], f_int[n * 3 + 2]);
            float3 f_external = nodes[nodeIdx].force / 6.0f;

            // delta = (Fext - Fint) * dt / mass 
            float3 deltaU = (f_external - f_internal) * relaxationFactor * rcp(mass);
            // float3 deltaU = float3(-f_int[n * 3 + 0], -f_int[n * 3 + 1], -f_int[n * 3 + 2]) * relaxationFactor * rcp(mass);

            if ((constraints & 1) != 0)
                deltaU.x = 0;
            if ((constraints & 2) != 0)
                deltaU.y = 0;
            if ((constraints & 4) != 0)
                deltaU.z = 0;

            InterlockedAdd(nodes[nodeIdx].displacement.x, deltaU.x);
            InterlockedAdd(nodes[nodeIdx].displacement.y, deltaU.y);
            InterlockedAdd(nodes[nodeIdx].displacement.z, deltaU.z);
        }
    }
    float vonMisesStress = sqrt(
        0.5f * ((stress[0] - stress[1]) * (stress[0] - stress[1]) + (stress[1] - stress[2]) * (stress[1] - stress[2]) +
                (stress[2] - stress[0]) * (stress[2] - stress[0])) +
        3.0f * (stress[3] * stress[3] + stress[4] * stress[4] + stress[5] * stress[5]));

    for (int idx = 0; idx < 4; ++idx)
    {
        InterlockedAdd(nodes[element.nodeIdxs[idx]].vonMisesStress, vonMisesStress);
        InterlockedAdd(nodes[element.nodeIdxs[idx]].stressCount, 1);
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

    var node = nodes[indices[vertexID]];
    var pos = node.position + node.displacement;

    float3 barycentrics[3] =
        {
            float3(1, 0, 0),
            float3(0, 1, 0),
            float3(0, 0, 1)
        };

    VSOutput output;
    output.position = mul(sceneViewData.viewProj, float4(pos, 1.0));
    output.color = StressToColor(node.vonMisesStress * rcp(node.stressCount), 10, 500);
    output.baryCoord = barycentrics[vertexID % 3];

    return output;
}

[require(fragment, spirv_1_6)]
[shader("fragment")]
PSOutput fragMain(VSOutput input)
{
    PSOutput output;

    var d = fwidth(input.baryCoord);
    var edge = smoothstep(0.0, d * 1.5, input.baryCoord);
    var edgeFactor = 1.0 - min(edge.x, min(edge.y, edge.z));
    var edgeColor = float3(0); // black

    var color = lerp(input.color, edgeColor, edgeFactor);
    output.colorOutput = float4(color, 1);

    return output;
}