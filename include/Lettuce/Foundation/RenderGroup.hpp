//
// Created by piero on 08/09/2024.
//
#pragma once
#include <vector>
#include <functional>
#include <string>
#include "Attachment.hpp"
#include "Resource.hpp"

namespace Lettuce::Foundation
{
    enum class ResourceUse
    {
        ReadOnly,
        WriteOnly,
        ReadWrite,
    };

    class RenderGroup
    {
    private:
        std::string _name;
        bool released = true;
        std::vector<Attachment> attachments;
        std::vector<Resource> resources;
        std::vector<ResourceUse> uses;

    public:
        RenderGroup(std::string name, BindPoint );
        void AddAttachment(Attachment attachment);
        void AddResource(Resource resource, ResourceUse use);
        void Release();
    };
}