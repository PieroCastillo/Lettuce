//
// Created by piero on 8/03/2025.
//
#pragma once

namespace Lettuce::Core
{
    class IReleasable
    {
    public:
        virtual void Release() = 0;
    };
}