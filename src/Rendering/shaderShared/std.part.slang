implementing LettuceRendering;

__include "std.constants.part.slang";
__include "std.deferred.part.slang";
__include "std.lighting.aux.part.slang";
__include "std.lighting.csm.part.slang";
__include "std.lighting.types.part.slang";
__include "std.ligthing.part.slang";
__include "std.materials.part.slang";
__include "std.mesh.part.slang";
__include "std.packing.part.slang";
__include "std.procedural.part.slang";
__include "std.quat.part.slang";
__include "std.scene.part.slang";

namespace std
{
Ptr<T> loadAddr<T>(uint64_t addr, uint32_t offset = 0)
{
    return reinterpret<Ptr<T>>(addr + (offset * sizeof(T)));
}

struct VkDrawIndirectCommand
{
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct IndirectCommandStorage
{
    uint32_t count;
    VkDrawIndirectCommand commands[];
};
};