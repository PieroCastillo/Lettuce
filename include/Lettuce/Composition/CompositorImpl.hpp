/*
Created by @PieroCastillo on 2026-01-27
*/
#ifndef LETTUCE_COMPOSITION_COMPOSITOR_IMPL_HPP
#define LETTUCE_COMPOSITION_COMPOSITOR_IMPL_HPP

// standard headers
#include <vector>

// project headers
#include "api.hpp"

namespace Lettuce::Composition
{
    /*
    Before all, we need to define behaviours:
    - Application send property changes by SetObjectProperty()
    - When Application call Commit(), the last changes per Visual-Property pair are accepted
      and put into a internal "values" queue
    - after update every visual/light/brush/effect, Compositor execute the Pipeline
    - Pipeline consist in 3 phases:
        - Objects  Pass: Writes BrushDataID, EffectDataID, LightDataID to ObjectDataTarget
        - Brushes  Pass
        - Lighting Pass
        - Effects  Pass
    - Application MUST proveed a Target for blenging pass
    */
    class CompositorImpl
    {
    private:
    public:
    };
};

#endif // LETTUCE_COMPOSITION_COMPOSITOR_IMPL_HPP