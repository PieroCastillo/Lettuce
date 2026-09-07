implementing LettuceRendering;

/*
why uint32_t instead of uint16 and uint64?
gpus like a lot reading memory using the same bit count per data element
so if I mix uint16 and uint64 read this may cause really annoying and debug-hard bugs
(i'm talking to you, AMD graphics cards >:D)
*/

struct MaterialEntry
{
    uint32_t typeIndex;
};

struct MaterialTable
{
    uint32_t pbrEntriesAddrHi;
    uint32_t pbrEntriesAddrLo;
    MaterialEntry entries[];
};