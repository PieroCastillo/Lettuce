[SpecializationConstant] const uint fs_VertexCount = 128;
[SpecializationConstant] const uint fs_TriangleCount = 256;
[SpecializationConstant] const uint fs_Invocations = 32;

[shader("mesh")]
[numthreads(fs_Invocations, 1, 1)]
void main(uint3 gid : SV_GroupThreadID)
{

}