/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_GPU_HPP
#define LETTUCE_CORE_GPU_HPP 

// standard headers
#include <optional>
#include <vector>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    class GPU
    {
        std::string name;
        bool isDedicated;
    };
}
#endif // LETTUCE_CORE_GPU_HPP