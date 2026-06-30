implementing LettuceQuimera;

/* lib structs */

struct SurfaceData
{
    uint32_t width;
    uint32_t height;
    uint32_t drawCommandCount;
    uint32_t reserved1;
};

struct ImplicitGeometryStorage
{
    float w, h;
    float ctl, ctr, cbl, cbr; // corners
};

struct SolidColorBrushStorage
{
    float4 color;
};

struct LayoutStorage
{
    float2 position;
    float2 scale;
    float2 skew;
    float2 anchorPoint;
    float rotation;
};

struct DrawCommand
{
    uint32_t layoutIdx;
    uint32_t geometryIdx;
    uint32_t brushIdx;
    uint32_t zOrder;
    // [ 1 bit (clipped) | 1 bit (ignoreTransform) | ... | 4 bit (effect heap) | 4 bit (brush heap) | 4 bit (geometry heap)]
    uint32_t flags;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
};

// shader only structs
struct TileMemory
{
    uint32_t drawCommandId;
};

struct PushConstants
{
    uint64_t surfaceDataAddr;
    uint64_t scratchTransformsAddr;
    uint64_t drawCommandsAddr;
    uint64_t layoutsAddr;
    uint64_t implicitGeometriesAddr;
    uint64_t solidColorBrushesAddr;
};
