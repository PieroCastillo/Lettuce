//
// Created by piero on 6/03/2025.
//
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "Utils.hpp"

namespace Lettuce::Core
{
    class IndirectCommandsLayout
    {
    public:
        VkIndirectCommandsLayoutEXT _commandsLayout;
        std::shared_ptr<Device> _device;

        IndirectCommandsLayout(const std::shared_ptr<Device> &device) : _device(device) {}
        void Assemble();
    };
}