//
// Created by piero on 14/10/2024.
//
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Lettuce/Core/common.hpp"
#include "Lettuce/X2D/RenderContext.hpp"

using namespace Lettuce::X2D;

//use subpasses
// use secondary command buffers and record with multi threading

RenderContext::RenderContext() {}
void RenderContext::Record(VkCommandBuffer cmd) {}