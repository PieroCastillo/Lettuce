/*
Created by @PieroCastillo on 2025-11-11
*/
#ifndef LETTUCE_CORE_SEQUENTIAL_EXECUTION_CONTEXT_HPP
#define LETTUCE_CORE_SEQUENTIAL_EXECUTION_CONTEXT_HPP

// project headers
#include "common.hpp"
#include "CommandRecordingContext.hpp"

namespace Lettuce::Core
{
    struct SequentialExecutionContextCreateInfo
    {

    };

    class SequentialExecutionContext
    {
        CommandRecordingContext m_cmdContext;
        uint64_t m_currentWaitValue;

        void record(VkCommandBuffer cmd, VkPipeline& currentPipeline, VkDeviceAddress& currentDescriptorBufferAddress, const Command& command);
    public:
        VkDevice m_device;
        VkQueue m_queue;
        VkCommandPool m_cmdPool;
        VkCommandBuffer m_cmd;
        VkSemaphore m_renderFinished;

        void Create(const IDevice& device, const SequentialExecutionContextCreateInfo& createInfo);
        void Release();

        CommandRecordingContext& GetCommandList();
        void Execute();
        void Wait();
    };
};
#endif // LETTUCE_CORE_SEQUENTIAL_EXECUTION_CONTEXT_HPP