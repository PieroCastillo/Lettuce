//
// Created by piero on 17/04/2025.
//
#pragma once

#include <type_traits>
#include <concepts>
#include <vector>

template <typename T>
concept VulkanHandle = std::is_same_v<T, uint64_t> ||
                       (std::is_pointer_v<T> &&
                        std::is_class_v<std::remove_pointer_t<T>>);

template <VulkanHandle T>
class IManageHandle
{
protected:
    T handle = VK_NULL_HANDLE;

public:
    T GetHandle() const
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
    const std::vector<T> &GetHandles() const
    {
        return handles;
    }

    void AddHandle(T &handle)
    {
        handles.push_back(std::move(handle));
    }

    T *GetHandlesPtrs()
    {
        return handles.data();
    }

    uint32_t GetCount() const
    {
        return (uint32_t)handles.size();
    }
};