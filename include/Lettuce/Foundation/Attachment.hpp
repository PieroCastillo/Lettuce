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
        bool released = true;
        TextureView &_view;
        AttachmentType _type;
        LoadOp _load;
        StoreOp _store;

    public:
        Attachment(std::string name, TextureView view, AttachmentType type, LoadOp load = LoadOp::Clear, StoreOp store = StoreOp::Store);
        void Release();
    };
}