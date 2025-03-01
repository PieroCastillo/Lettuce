//
// Created by piero on 28/02/2025.
//
#pragma once

namespace Lettuce::Foundation
{
    struct IWorkUnit
    {
    public:
        virtual void Record(VkCommandBuffer cmd);
    };
};