//
// Created by piero on 08/09/2024.
//
#pragma once
#include <vector>
#include <functional>
#include <string>

namespace Lettuce::Foundation
{
    class Resource
    {
    private:
        int index;
        int version;

    public:
        Resource(std::string name, Sampler sampler, TextureView view);
        Resource(std::string name, Buffer buffer);
    };
}