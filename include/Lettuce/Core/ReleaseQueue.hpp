//
// Created by piero on 8/03/2025.
//
#pragma once
#include <vector>
#include <memory>
#include <ranges>
#include <tuple>
#include <functional>
#include "IReleasable.hpp"

namespace Lettuce::Core
{
    class ReleaseQueue
    {
    private:
        std::vector<std::tuple<std::shared_ptr<IReleasable>, std::function<void(void)>>> list;

    public:
        void PushWithBefore(const std::shared_ptr<IReleasable> &releasable, std::function<void(void)> before)
        {
            list.push_back({releasable, before});
        }
        void Push(const std::shared_ptr<IReleasable> &releasable)
        {
            auto nothing = []() {};
            list.push_back({releasable, nothing});
        }
        void ReleaseAll()
        {
            for (auto &[element, func] : std::views::reverse(list))
            {
                func();
                element->Release();
            }
        }
    };
}