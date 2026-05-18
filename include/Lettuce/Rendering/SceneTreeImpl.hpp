/*
Created by @PieroCastillo on 2026-05-17
*/
#ifndef LETTUCE_RENDERING_SCENE_TREE_IMPL_HPP
#define LETTUCE_RENDERING_SCENE_TREE_IMPL_HPP

#include "../Core/api.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Rendering
{
    struct SceneTreeImpl
    {
        Device* device = nullptr;

       void Create(const SceneTreeDesc& desc);
       void Destroy();
    };
};

#endif // LETTUCE_RENDERING_SCENE_TREE_IMPL_HPP