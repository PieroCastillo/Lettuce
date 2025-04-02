//
// Created by piero on 1/04/2025.
//
#pragma once
#include <vector>
#include <memory>
#include <variant>
#include <string>
#include <utility>

#include "Lettuce/Core/ImageViewResource.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Foundation
{
    enum class RenderTargetViewType
    {
        Color,
        DepthStencil,
        Input,
    };

    class RenderTargetView
    {
    private:
        friend class RenderDispatcher;

        std::string _name;
        std::shared_ptr<ImageViewResource> _imageView;

    public:
        explicit RenderTargetView(std::string &name,
                                  std::shared_ptr<ImageViewResource> imageView)
            : _name(name), _imageView(imageView)
        {
        }
    };
}