//
// Created by piero on 13/11/2024.
//
#include <iostream>
#include "Lettuce/X3D/Mesh.hpp"

void Lettuce::X3D::Mesh::setup()
{
    check();
    
}

void Lettuce::X3D::Mesh::check()
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

void Lettuce::X3D::Mesh::LoadASCIIFromFile(std::string fileName)
{
    result = loader.LoadASCIIFromFile(&model, &err, &warn, fileName);
}
void Lettuce::X3D::Mesh::LoadASCIIFromString(std::string data, std::string baseDir)
{
    result = loader.LoadASCIIFromString(&model, &err, &warn, data.c_str(), data.length(), baseDir);
}
void Lettuce::X3D::Mesh::LoadBinaryFromFile(const std::string fileName)
{
    result = loader.LoadBinaryFromFile(&model, &err, &warn, fileName);
}
void Lettuce::X3D::Mesh::LoadBinaryFromMemory(unsigned char *data, unsigned int size, std::string baseDir)
{
    result = loader.LoadBinaryFromMemory(&model, &err, &warn, data, size, baseDir);
}

void Lettuce::X3D::Mesh::Release()
{
    model.~Model();
}