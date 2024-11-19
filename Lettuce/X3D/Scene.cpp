//
// Created by piero on 18/11/2024.
//
#include <iostream>
#include "Lettuce/X3D/Scene.hpp"

void Lettuce::X3D::Scene::setup()
{
    check();
    
}

void Lettuce::X3D::Scene::check()
{
    if (!warn.empty())
    {
        std::cout << "Warn: " << warn << std::endl;
    }

    if (!err.empty())
    {
        std::cout << "Error: " << err << std::endl;
    }

    if (!result)
    {
        std::cout << "Failed to parse glTF" << std::endl;
        return;
    }
}

void Lettuce::X3D::Scene::LoadASCIIFromFile(std::string fileName)
{
    result = loader.LoadASCIIFromFile(&model, &err, &warn, fileName);
}
void Lettuce::X3D::Scene::LoadASCIIFromString(std::string data, std::string baseDir)
{
    result = loader.LoadASCIIFromString(&model, &err, &warn, data.c_str(), data.length(), baseDir);
}
void Lettuce::X3D::Scene::LoadBinaryFromFile(const std::string fileName)
{
    result = loader.LoadBinaryFromFile(&model, &err, &warn, fileName);
}
void Lettuce::X3D::Scene::LoadBinaryFromMemory(unsigned char *data, unsigned int size, std::string baseDir)
{
    result = loader.LoadBinaryFromMemory(&model, &err, &warn, data, size, baseDir);
}

void Lettuce::X3D::Scene::Release()
{
    model.~Model();
}