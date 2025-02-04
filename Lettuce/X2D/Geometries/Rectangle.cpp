//
// Created by piero on 14/10/2024.
//
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Lettuce/Core/common.hpp"
#include "Lettuce/X2D/Geometries/Rectangle.hpp"
#include "Lettuce/X2D/Geometries/Vertex.hpp"

using namespace Lettuce::X2D::Geometries;

Rectangle::Rectangle(const std::shared_ptr<Device> &device)
{
    indexBuffer = Buffer::CreateIndexBuffer<uint32_t>(device, {0,1,3,1,2,3});
    vertexBuffer = Buffer::CreateVertexBuffer<Vertex>(device, {
        {{},{}},
        {{},{}},
        {{},{}},
        {{},{}},
    });
}
void Rectangle::SetSizeAndCenter(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{

}