/*
Created by @PieroCastillo on 2025-01-07
*/
#ifndef LETTUCE_CORE_FORMATS_HPP
#define LETTUCE_CORE_FORMATS_HPP

#include <cstdint>
namespace Lettuce::Core
{
    enum class Format : uint8_t
    {
        Undefined = 0,
        // 8 bit component
        Raw_R8_UInt = 13,
        Raw_R8_SInt = 14,
        Raw_RG8_UInt = 20,
        Raw_RG8_SInt = 21,
        Raw_RGBA8_UInt = 41,
        Raw_RGBA8_SInt = 42,
        Raw_R8_UNorm = 9,
        Raw_R8_SNorm = 10,
        Raw_RG8_UNorm = 16,
        Raw_RG8_SNorm = 17,
        Raw_RGBA8_UNorm = 37,
        Raw_RGBA8_SNorm = 38,
        // 16 bit component
        Raw_R16_UInt = 74,
        Raw_R16_SInt = 75,
        Raw_RG16_UInt = 81,
        Raw_RG16_SInt = 82,
        Raw_RGBA16_UInt = 95,
        Raw_RGBA16_SInt = 96,
        Raw_R16_UNorm = 70,
        Raw_R16_SNorm = 71,
        Raw_RG16_UNorm = 77,
        Raw_RG16_SNorm = 78,
        Raw_RGBA16_UNorm = 91,
        Raw_RGBA16_SNorm = 92,
        Raw_R16_SFloat = 76,
        Raw_RG16_SFloat = 83,
        Raw_RGBA16_SFloat = 97,
        // 32 bit component
        Raw_R32_UInt = 98,
        Raw_R32_SInt = 99,
        Raw_RG32_UInt = 101,
        Raw_RG32_SInt = 102,
        Raw_RGBA32_UInt = 107,
        Raw_RGBA32_SInt = 108,
        Raw_R32_SFloat = 100,
        Raw_RG32_SFloat = 103,
        Raw_RGBA32_SFloat = 109,
        // sampled/compressed: 
        Compressed_BC1_RGB_UNorm = 131,
        Compressed_BC1_RGB_sRGB = 132,
        Compressed_BC1_RGBA_UNorm = 133,
        Compressed_BC1_RGBA_sRGB = 134,
        Compressed_BC2_UNorm = 135,
        Compressed_BC2_sRGB = 136,
        Compressed_BC3_UNorm = 137,
        Compressed_BC3_sRGB = 138,
        Compressed_BC4_UNorm = 139,
        Compressed_BC4_SNorm = 140,
        Compressed_BC5_UNorm = 141,
        Compressed_BC5_SNorm = 142,
        Compressed_BC6H_UFloat = 143,
        Compressed_BC6H_SFloat = 144,
        Compressed_BC7_UNorm = 145,
        Compressed_BC7_sRGB = 146,
        // universal surface:
        UniversalSurface_BGRA8_UNorm = 44,
        UniversalSurface_BGRA8_sRGB = 50,
        // universal depth stencil: 
        Universal_DepthStencil_D32_SFloat_S8_UInt = 130,
        // storage w/ atomic: 
        Atomic_R32_UInt = 98,
        Atomic_R32_SInt = 99,
        Atomic_R32_SFloat = 100,
        Atomic_R64_UInt = 110,
        Atomic_R64_SInt = 111,
        // common packed (sampled/storage) 
        Raw_A2R10G10B10_UNorm_Pack32 = 58,
        Raw_A2B10G10R10_UNorm_Pack32 = 64,
        Raw_A2B10G10R10_UInt_Pack32 = 68,
        Raw_B10G11R11_UFloat_Pack32 = 122,
        Raw_E5B9G9R9_UFloat_Pack32 = 123,
        Raw_A8B8G8R8_UNorm_Pack32 = 51,
        Raw_A8B8G8R8_UInt_Pack32 = 55
    };
}
#endif // LETTUCE_CORE_FORMATS_HPP