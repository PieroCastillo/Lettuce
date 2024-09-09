//
// Created by piero on 22/08/2024.
//
#pragma once
#include <vector>
#include <functional>
#include "Core/CommandList.hpp"

namespace Lettuce::Foundation
{
    enum class ResourceUse
    {
        ReadOnly,
        WriteOnly,
        ReadWrite,
    };

    class RenderGraphImpl
    {
    public:
        VkCommandPool pool;
        std::vector<VkCommandBuffer> cmdNodes;
        RenderGraphImpl(uint32_t maxNodes);
    };

    /// @brief RenderGraph is a Directed Acyclic Graph (DAG)
    /// their main purpose is automatize the parallelization
    /// and the synchronization of the rendering and
    /// computing tasks.
    /// Task Graphs use secondary command buffers for each node
    class RenderGraph
    {
    private:
        RenderGraphImpl impl;
        bool opened = false;
        bool isAcyclic();
        void buildSync();

    public:
        RenderGraph(uint32_t maxNodes = 32);
        void Open();

        void Close();
        void Run();
    };
}