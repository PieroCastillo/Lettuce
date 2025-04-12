//
// Created by piero on 7/04/2025.
//
#include <iostream>

import Renderer;

using namespace Lettuce::Renderer::Core;

void initialize() {}
void createDevice() {}
void createResource1() {}
void createResource2() {}
void draw() {}
void release() {}

int main()
{
    std::cout << "Hello World! " << std::endl;
    // create task graph
    // register func
    // load modules
    // execute graph

    TaskGraph graph;

    auto [t1, t2, t3, t4, t5, t6] = graph.Emplace(
        { ExecutionMode::OneTime, initialize, []() {return true;}},
        { ExecutionMode::OneTime, createDevice, []() {return true;}},
        { ExecutionMode::OneTime, createResource1, []() {return true;}},
        { ExecutionMode::OneTime, createResource2, []() {return true;}},
        { ExecutionMode::Continuous, draw, []() {return glfwPoolEvents(window*);}},
        { ExecutionMode::OneTime, release, []() {return true;}});

    t1.Next(t2).Next(t3, t4).Next(t5).Next(t6);

    graph.DispatchRoot(t1);
}