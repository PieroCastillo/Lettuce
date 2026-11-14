/*
Created by @PieroCastillo on 2026-1-13
*/
#ifndef LETTUCE_RENDERING_BASE_PASS_HPP
#define LETTUCE_RENDERING_BASE_PASS_HPP

#include "../Core/api.hpp"
#include "../Foundations/api.hpp"
#include "types.hpp"

using namespace Lettuce::Foundations;

namespace Lettuce::Rendering
{
    class BasePass
    {
    protected:
        Device* m_device = nullptr;
        DescriptorTable m_descriptorTable;
    public:
        BasePass() = default;
        BasePass(const BasePass&) = delete;
        BasePass& operator=(const BasePass&) = delete;
    };
};
#endif // LETTUCE_RENDERING_BASE_PASS_HPP