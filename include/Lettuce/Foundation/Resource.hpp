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
        bool released = true;
        bool isBuffer = false;
        std::string _name;
        Sampler &_sampler;
        TextureView &_view;
        Buffer &_buffer;

    public:
        int index = 0;
        int version = 0;
        Resource(std::string name, Sampler sampler, TextureView view);
        Resource(std::string name, Buffer buffer);
        void Release();
    };
}