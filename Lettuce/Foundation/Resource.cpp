//
// Created by piero on 08/09/2024.
//
#include <vector>
#include <functional>
#include <string>
#include "Lettuce/Core/Sampler.hpp"
#include "Lettuce/Core/TextureView.hpp"
#include "Lettuce/Core/Buffer.hpp"
#include "Lettuce/Foundation/Resource.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Foundation;

Resource::Resource(std::string name, Sampler sampler, TextureView view)
{
    if (!released) 
    {
        throw std::runtime_error("resource with name: " + name + " didn't have been released!");
    }
    _name = name;
    isBuffer = false;
    _sampler = sampler;
    _view = view;
}
Resource::Resource(std::string name, Buffer buffer)
{
    if (!released)
    {
        throw std::runtime_error("resource with name: " + name + " didn't have been released!");
    }
    _name = name;
    isBuffer = true;
    _buffer = buffer;
}
void Resource::Release()
{
    released = true;
    _name = "";
    index = 0;
    version = 0;
}