//
// Created by piero on 22/08/2024.
//
// #include <vector>
// #include <functional>
// #include <volk.h>
// #include "CommandList.hpp"

// #pragma once
// namespace Lettuce::Core
// {

//     /// @brief TaskGraph is a Directed Acyclic Graph (DAG)
//     /// their main purpose is automatize the parallelization
//     /// and the synchronization of the rendering and
//     /// computing tasks.
//     class TaskGraph
//     {
//     private:
//         VkCommandPool _commandPool;
//         bool opened = false;
//         bool isAcyclic();
//         void buildSync();

//     public:
//         std::vector<VkCommandBuffer> cmdNodes;
//         TaskGraph(uint32_t maxNodes = 32);
//         void Open();
//         void Close();
//         void AttachNode(TaskNode node);
//         void NodeBeforeNode(TaskNode node1, TaskNode node2);
//         void Run();
//     };

//     class TaskNode
//     {
//     private:
//         std::function<void(CommandList)> func;
//         std::vector<TaskNode> parents;

//     public:
//         int id;
//         TaskNode(std::function<void(CommandList)> task);
//     };
// }