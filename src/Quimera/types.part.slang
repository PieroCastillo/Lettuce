implementing LettuceQuimera;

/* lib structs */

struct ImplicitGeometryStorage
{
    float x, y, w, h;
    float ctl, ctr, cbl, cbr; // corners
};

struct SolidColorBrushStorage
{
    float4 color;
};

struct DrawCommand
{
    uint32_t transformIdx;
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
    uint64_t drawCommandsAddr;
    uint64_t transformsAddr;
    uint64_t implicitGeometriesAddr;
    uint64_t solidColorBrushesAddr;
};