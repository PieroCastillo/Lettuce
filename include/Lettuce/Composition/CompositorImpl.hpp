/*
Created by @PieroCastillo on 2026-01-27
*/
#ifndef LETTUCE_COMPOSITION_COMPOSITOR_IMPL_HPP
#define LETTUCE_COMPOSITION_COMPOSITOR_IMPL_HPP

// standard headers
#include <vector>

// project headers
#include "api.hpp"
#include "../Core/Allocators/LinearAllocator.hpp"
#include "../Core/ResourcePool.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Composition
{
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
    private:
        // Lettuce Objects
        Device device;
        CommandAllocator cmdAlloc;
        
        Allocators::LinearAllocator memAlloc;
        MemoryView queuedAnimationsView;
        MemoryView animationsView;
        MemoryView visualsView;
        MemoryView brushesView;
        MemoryView lightsView;
        MemoryView effectsView;
        RenderTarget objectsTarget;  // BrushDataID/EffectDataID/LightDataID per pixel
        RenderTarget renderTarget;   // EffectPass writes here

        DescriptorTable descriptorTable;
        Pipeline pAnimationEvaluationPass;
        Pipeline pObjectPass;
        Pipeline pBrushesPass;
        Pipeline pLightingPass;
        Pipeline pEffectPass;
    public:
    };
};

#endif // LETTUCE_COMPOSITION_COMPOSITOR_IMPL_HPP