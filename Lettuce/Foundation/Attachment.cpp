//
// Created by piero on 08/09/2024.
//
#include <vector>
#include <functional>
#include <string>
#include "Lettuce/Core/common.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/TextureView.hpp"
#include "Lettuce/Core/Sampler.hpp"
#include "Lettuce/Foundation/Attachment.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Foundation;

Attachment::Attachment(std::string name, TextureView view, AttachmentType type, LoadOp load = LoadOp::Clear, StoreOp store = StoreOp::Store)
{
    if (!released)
    {
        throw std::runtime_error("attachment with name: " + name + " didn't have been released!");
    }
    _name = name;
    _view = view;
    _type = type;
    _load = load;
    _store = store;
}
void Attachment::Release()
{
    released = true;
    _name = "";
}
