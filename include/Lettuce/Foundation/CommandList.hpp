//
// Created by piero on 1/04/2025.
//
#pragma once
#include <vector>
#include <memory>
#include <variant>
#include <string>
#include <utility>
#include <vector>

#include "CommandState.hpp"
#include "CommandData.hpp"
#include "RenderTask.hpp"

namespace Lettuce::Foundation
{
    class CommandList
    {
    private:
        std::vector<RenderTask> tasks;

    public:
        void AddTask(RenderTask &&renderTask)
        {
            tasks.emplace_back(std::forward<RenderTask>(renderTask));
        }
    };
}