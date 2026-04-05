/*
Created by @PieroCastillo on 2026-01-27
*/
#ifndef LETTUCE_COMPOSITION_COMPOSITOR_IMPL_HPP
#define LETTUCE_COMPOSITION_COMPOSITOR_IMPL_HPP

// standard headers
#include <atomic>
#include <mutex>
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
        Material,
        Light,
    };

    /*
    Before all, we need to define behaviours:
    - Application send property changes by SetObjectProperty()
    - When Application call Commit(), the last changes per Visual-Property pair are accepted
      and put into a internal "values" queue
    - after update every visual/light/Material/effect, Compositor execute the Pipeline
    - Pipeline consist in 3 phases:
        - AnimationEvaluationPass
        - Objects  Pass: Writes MaterialDataID, EffectDataID, LightDataID to ObjectDataTarget
        - Materiales  Pass
        - Lighting Pass
    - Application MUST proveed a Target for blending pass
    */
    class CompositorImpl
    {
    public:
        // Lettuce Objects
        Device* device;
        Swapchain swapchain;
        
        CommandAllocator cmdAlloc;
        uint32_t maxVisuals;
        uint32_t maxMaterials;
        uint32_t maxLights;
        uint32_t maxAnimations;
        uint32_t maxLinkedTextures;

        Allocators::LinearAllocator memAlloc;
        MemoryView queuedAnimationsView;
        MemoryView animationsView;
        MemoryView visualsView;
        MemoryView MaterialesView;
        MemoryView lightsView;
        Allocators::LinearAllocator memTargetsAlloc;
        RenderTarget objectsTarget;  // MaterialDataID/LightDataID per pixel
        RenderTarget renderTarget;   // Composing Material Pass writes here

        DescriptorTable descriptorTable;
        Pipeline pTileBinningPass;
        Pipeline pTileRasterPass;
        Pipeline pPostprocessingPass;

        // Resource Pools
        ResourcePool<Visual, VisualInfo> visuals;
        ResourcePool<Material, MaterialInfo> materials;
        ResourcePool<Light, LightInfo> lights;
        ResourcePool<AnimationToken, AnimationTokenInfo> animationTokens;

        std::jthread compositorThread;
        std::stop_token compositorStopToken;

        std::mutex commitMutex;
        CommandQueue appQueue;
        std::vector<Command> compositorQueue;

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