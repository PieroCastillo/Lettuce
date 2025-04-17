//
// Created by piero on 17/04/2025.
//
#pragma once

#include <type_traits>
#include <vector>

template <typename T>
concept VulkanHandle = std::is_base_of<uint64_t, T>::value;

template <VulkanHandle T>
class IManageHandle
{
protected:
    T handle;

public:
    T GetHandle()
    {
        return handle;
    }

    T *GetHandlePtr()
    {
        return &handle;
    }
};

template <VulkanHandle T>
class IManageHandleGroup
{
protected:
    std::vector<T> handles;

public:
    std::vector<T> GetHandles()
    {
        return handles;
    }

    T *GetHandlesPtrs()
    {
        return handles.data();
    }

    uint32_t GetCount()
    {
        return (uint32_t)handles.size();
    }
};