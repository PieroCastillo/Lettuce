/*
Created by @PieroCastillo on 2026-08-11
*/
#ifndef LETTUCE_UTILS_ALGORITHM_HPP
#define LETTUCE_UTILS_ALGORITHM_HPP

#include "../Core/api.hpp"
#include "../Rendering/api.hpp"
#include "../Foundations/api.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Foundations;

namespace Lettuce::Utils
{
    class Algorithm
    {
    public:
        template<GpuElement T>
        static void LazyCopyPixel(CommandBuffer& cmd, TextureView srcTexture, uint32_t x, uint32_t y, GpuUniquePtr<T>& dst)
        {
            TextureToMemoryCopy tmPixelCopy =
            {
                .srcTexture = srcTexture,
                .dstMemory = dst.getView(),
                .mipmapLevel = 0,
                .layerBaseLevel = 0,
                .layerCount = 1,
                .x = static_cast<uint32_t>(x), .y = static_cast<uint32_t>(y), .width = 1, .height = 1,
            };

            cmd.MemoryCopy(tmPixelCopy);
        }
    };
};
#endif // LETTUCE_UTILS_ALGORITHM_HPP