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

namespace Lettuce::Foundation
{
    template <CommandData T>
    struct RenderTaskGeneric
    {
        std::string _name;
        std::vector<CommandState> _states;
        std::vector<T> _commands;

        RenderTaskGeneric(std::string name,
                          std::vector<CommandState> states,
                          std::vector<T> commands)
            : _name(name), _states(states), _commands(commands)
        {
        }
    };

    using RenderTask = std::variant<RenderTaskGeneric<DrawCommandData>,
                                    RenderTaskGeneric<DrawIndexedCommandData>,
                                    RenderTaskGeneric<DrawIndirectCommandData>,
                                    RenderTaskGeneric<DrawIndexedIndirectCommandData>,
                                    RenderTaskGeneric<DrawIndirectCountCommandData>,
                                    RenderTaskGeneric<DrawIndexedIndirectCountCommandData>,
                                    RenderTaskGeneric<DrawMeshTasksCommandData>,
                                    RenderTaskGeneric<DrawMeshTasksIndirectCommandData>,
                                    RenderTaskGeneric<DrawMeshTasksIndirectCountCommandData>,
                                    RenderTaskGeneric<DispatchCommandData>,
                                    RenderTaskGeneric<DispatchIndirectCommandData>>;
}