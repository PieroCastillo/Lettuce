/*
Created by @PieroCastillo on 2025-12-28
*/
#ifndef LETTUCE_CORE_COMMAND_BUFFER_IMPL_HPP
#define LETTUCE_CORE_COMMAND_BUFFER_IMPL_HPP

// standard headers
#include <concepts>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

// project headers
#include "api.hpp"
#include "common.hpp"
#include "HelperStructs.hpp"
#include "ResourcePool.hpp"
#include "DeviceImpl.hpp"

namespace Lettuce::Core
{
    struct CommandBufferImpl
    {
        DeviceImpl* device;
        VkCommandBuffer vkCmd;
    };
};
#endif // LETTUCE_CORE_COMMAND_BUFFER_IMPL_HPP