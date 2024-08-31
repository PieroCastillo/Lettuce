//
// Created by piero on 22/08/2024.
//
// #include "Lettuce/Core/common.hpp"
// #include <thread>
// #include "Lettuce/Core/TaskGraph.hpp"
// #include "Lettuce/Core/CommandList.hpp"
// #include "Lettuce/Core/Utils.hpp"

// using namespace Lettuce::Core;

// void TaskNode::Execute(VkCommandBufferInheritanceInfo cmdII)
// {
//     auto cmd = graph.cmdNodes[this->id];
//     // begin
//     VkCommandBufferBeginInfo cmdBI = {
//         .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
//         .pInheritanceInfo = &cmdII,
//     };
//     checkResult(vkBeginCommandBuffer(cmd, &cmdBI));
//     CommandList cmdList;
//     cmdList._commandBuffer = cmd;
//     this->func(cmdList);
//     checkResult(vkEndCommandBuffer(cmd));
// }