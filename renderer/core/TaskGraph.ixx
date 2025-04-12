//
// Created by piero on 6/04/2025.
//

module;

// includes stay here

import std;

export module Renderer:Core.TaskGraph;
export import :Core.TaskNode;

export namespace Lettuce::Renderer::Core
{
    class TaskGraph
    {
        void executeNode(TaskNode &node)
        {
            // if the node is visited, do nothing and return
            if (node.visited)
                return;

            if (node.executionMode == ExecutionMode::OneTime && node.conditionF())
            {
                node.executeF();
                node.visited = true;
            }
            else if (node.executionMode == ExecutionMode::Continuous)
            {
                while (node.conditionF())
                {
                    node.executeF();
                }
            }
            else if (node.executionMode == ExecutionMode::OnDemand)
            {
                // TODO
                return;
            }

            std::vector<std::thread> threads;

            for (auto &child : node.children)
            {
                std::thread childThread(executeNode, child);
                childThread.detach();
                threads.push_back(childThread);
            }
        }

    public:
        std::thread executionThread;

        void DispatchRoot(TaskNode &root)
        {
            if (root.visited)
                return;

            executionThread = std::thread(executeNode, root);
        }

        template<typename... T>
        auto emplace(T&&... funcs) {
            size_t start = tasks_.size();
            tasks_.emplace_back(std::forward<Funcs>(funcs)), ...;
            return create_tuple(start, std::index_sequence_for<Funcs...>{});
        }

        void Wait()
        {
            executionThread.join();
        }
    };
}