/*
Created by @PieroCastillo on 2025-11-5
*/
#ifndef LETTUCE_CORE_RESOURCE_DICTIONARY_HPP
#define LETTUCE_CORE_RESOURCE_DICTIONARY_HPP

// standard headers
#include <string>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    class ResourceDictionary
    {
    public:
        void AddResource(const std::string& resName);
        void DeleteResource(const std::string& resName);

        void GetResource(const std::string& resName);
        void Reset();
    };
}

#endif // LETTUCE_CORE_RESOURCE_DICTIONARY_HPP