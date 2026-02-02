/*
Created by @PieroCastillo on 2026-01-27
*/
#ifndef LETTUCE_COMPOSITION_COMPOSITOR_IMPL_HPP
#define LETTUCE_COMPOSITION_COMPOSITOR_IMPL_HPP

// standard headers
#include <atomic>
#include <thread>
#include <vector>

// project headers
#include "api.hpp"
#include "HelperStructs.hpp"
#include "../Core/Allocators/LinearAllocator.hpp"
#include "../Core/ResourcePool.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Composition
{
    enum class AnimatableType
    {
        Visual,
        Brush,
        Light,
        Effect,
    };

    /*
    Before all, we need to define behaviours:
    - Application send property changes by SetObjectProperty()
    - When Application call Commit(), the last changes per Visual-Property pair are accepted
      and put into a internal "values" queue
    - after update every visual/light/brush/effect, Compositor execute the Pipeline
    - Pipeline consist in 3 phases:
        - AnimationEvaluationPass
        - Objects  Pass: Writes BrushDataID, EffectDataID, LightDataID to ObjectDataTarget
        - Brushes  Pass
        - Lighting Pass
        - Effects  Pass
    - Application MUST proveed a Target for blending pass
    */
    class CompositorImpl
    {
    public:
        // Lettuce Objects
        Device* device;
        CommandAllocator cmdAlloc;
        uint32_t maxVisuals;
        uint32_t maxBrushes;
        uint32_t maxLights;
        uint32_t maxEffects;
        uint32_t maxAnimations;
        uint32_t maxLinkedTextures;

        Allocators::LinearAllocator memAlloc;
        MemoryView queuedAnimationsView;
        MemoryView animationsView;
        MemoryView visualsView;
        MemoryView brushesView;
        MemoryView lightsView;
        MemoryView effectsView;
        Allocators::LinearAllocator memTargetsAlloc;
        RenderTarget objectsTarget;  // BrushDataID/EffectDataID/LightDataID per pixel
        RenderTarget renderTarget;   // EffectPass writes here

        DescriptorTable descriptorTable;
        Pipeline pAnimationEvaluationPass;
        Pipeline pTilesPass;
        Pipeline pRasterPass;

        // Resource Pools
        ResourcePool<Visual, VisualInfo> visuals;
        ResourcePool<Brush, BrushInfo> brushes;
        ResourcePool<Light, LightInfo> lights;
        ResourcePool<Effect, EffectInfo> effects;
        ResourcePool<AnimationToken, AnimationTokenInfo> animationTokens;

        // Synchronization Primitives
        std::atomic<bool> stop;
        std::thread compositorThread;

        uint32_t width;
        uint32_t height;

        void Create(const CompositorDesc& desc);
        void Destroy();

        void CreateResources();
        void CreatePipelines();

        void MainLoop();
    };
};

#endif // LETTUCE_COMPOSITION_COMPOSITOR_IMPL_HPP