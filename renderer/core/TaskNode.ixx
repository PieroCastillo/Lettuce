//
// Created by piero on 6/04/2025.
//

module;

// includes stay here
#include <functional>
#include <vector>

export module Renderer:Core.TaskNode;

export namespace Lettuce::Renderer::Core
{
    class TaskGraph;

    enum class ExecutionMode
    {
        OneTime,
        OnDemand,
        Continuous,
    };

    class TaskNode
    {
    protected:
        bool visited = false;
        friend class TaskGraph;
        ExecutionMode executionMode;
        std::function<void(void)> executeF;
        std::function<bool(void)> conditionF;

        std::vector<TaskNode> children;

        void execute()
        {
            if(conditionF())
                executeF();
        }

    public:
        TaskNode(ExecutionMode mode, std::function<void(void)> execute, std::function<bool(void)> condition)
            : executionMode(mode),
              executeF(execute),
              conditionF(condition)
        {
        }

        template<typename... Tasks>
        auto Next(Tasks&... tasks)
        {
            
        }

        void LinkTo(TaskNode &node)
        {
            children.push_back(node);
        }
    };
}