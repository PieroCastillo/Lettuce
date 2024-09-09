//
// Created by piero on 08/09/2024.
//
#pragma once
#include <vector>
#include <functional>
#include <string>

namespace Lettuce::Foundation
{
    class Attachment
    {
    private:
        std::string _name;

    public:
        Attachment(std::string name, TextureView view, AttachmentType type);
    };
}