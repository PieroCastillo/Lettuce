/*
Created by @PieroCastillo on 2026-11-13
*/
#ifndef LETTUCE_FOUNDATIONS_LOADER_HPP
#define LETTUCE_FOUNDATIONS_LOADER_HPP

#include <fstream>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "../Core/api.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Foundations
{
    class Loader
    {
    public:
        static auto LoadSpirv(Device* pDevice, std::string_view path) -> ShaderBinary;
    };
};
#endif // LETTUCE_FOUNDATIONS_LOADER_HPP