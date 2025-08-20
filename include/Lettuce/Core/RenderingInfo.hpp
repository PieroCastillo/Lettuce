/*
Created by @PieroCastillo on 2025-08-17
*/
#ifndef LETTUCE_CORE_RENDERING_INFO_HPP
#define LETTUCE_CORE_RENDERING_INFO_HPP

// standard headers
#include <vector>
#include <memory>

// project headers
#include "RenderTarget.hpp"

namespace Lettuce::Core {
    struct RenderingInfo {
        std::vector<std::shared_ptr<RenderTarget>> colorRenderTargets;
        std::shared_ptr<RenderTarget> depthRenderTarget;
        std::shared_ptr<RenderTarget> stencilRenderTarget;
    };
};
#endif // LETTUCE_CORE_RENDERING_INFO_HPP