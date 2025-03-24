//
// Created by piero on 22/03/2025.
//
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "WorkState.hpp"
#include "CommandDatas.hpp"
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/ImageResource.hpp"

namespace Lettuce::Foundation
{
    // NEEDS REVIEW

    struct WorkNode
    {
        std::vector<WorkState> states;
        std::vector<std::weak_ptr<WorkNode>> nextNodes;

        WorkNode(const std::string& name, CommandData data)
        {

        }

        void WritesTo(const std::shared_ptr<WorkNode> &node, const std::shared_ptr<BufferResource> &buffer)
        {

        }

        void WritesTo(const std::shared_ptr<WorkNode> &node, const std::shared_ptr<ImageResource> &image)
        {

        }
    };
}