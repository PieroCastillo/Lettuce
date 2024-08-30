//
// Created by piero on 22/08/2024.
//
#include <vector>
#include <functional>
#include <volk.h>
#include "CommandList.hpp"

#pragma once
namespace Lettuce::Core
{
    class TaskGraphImpl
    {
    public:
        VkCommandPool pool;
        std::vector<VkCommandBuffer> cmdNodes;
        TaskGraphImpl(uint32_t maxNodes);
    };

    /// @brief TaskGraph is a Directed Acyclic Graph (DAG)
    /// their main purpose is automatize the parallelization
    /// and the synchronization of the rendering and
    /// computing tasks.
    /// Task Graphs use secondary command buffers for each node
    class TaskGraph
    {
    private:
        TaskGraphImpl impl;
        bool opened = false;
        bool isAcyclic();
        void buildSync();

    public:
        TaskGraph(uint32_t maxNodes = 32);
        void Open();
        void Close();
        void AttachNode(TaskNode node);
        void Optimize();
        void Run();
    };

    class TaskNode
    {
    private:
        int id;

    public:
        TaskNode(std::function<void(CommandList)> task);
        void Read(Resource resource);
        void Write(Resource resource);
    };

    class Resource
    {
    private:
        int index;
        int version;

    public:
        Resource(Texture texture);
        Resource(Buffer buffer);
    };
}