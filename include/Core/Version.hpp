//
// Created by piero on 12/02/2024.
//
#include <cstdint>

namespace Lettuce::Core
{
    struct Version
    {
        uint32_t variant = 1;
        uint32_t major = 0;
        uint32_t minor = 0;
        uint32_t patch = 0;
    };
}