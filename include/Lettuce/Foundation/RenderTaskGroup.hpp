//
// Created by piero on 1/04/2025.
//
#pragma once
#include <vector>
#include <memory>
#include <variant>
#include <string>
#include <functional>
#include <utility>
#include <vector>

#include "CommandList.hpp"
#include "CommandState.hpp"
#include "CommandData.hpp"
#include "RenderTargetView.hpp"

namespace Lettuce::Foundation
{
    class RenderTaskGroup
    {
    private:
    public:
        void RenderTo(RenderTargetView &target, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp);
        void Record(std::function<void(CommandList)> func);
    };
}