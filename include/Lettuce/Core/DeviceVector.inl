/*
Created by @PieroCastillo on 2025-11-4
*/
#ifndef LETTUCE_CORE_DEVICE_VECTOR_INL
#define LETTUCE_CORE_DEVICE_VECTOR_INL

// project headers
#include "DeviceVector.hpp"

using namespace Lettuce::Core;

template<typename T>
void DeviceVector<T>::Create(const IDevice& device, const DeviceVectorCreateInfo& createInfo)
{
    base.Create(device, createInfo, sizeof(T));
}

template<typename T>
void DeviceVector<T>::Release()
{
    base.Release();
}

template<typename T>
void DeviceVector<T>::Push(const T& element)
{
    base.PushRange(*element, 1);
}

template<typename T>
void DeviceVector<T>::CopyFrom(const std::span<T>& dst)
{
    base.PushRange(dst.data(), (uint32_t)dst.size());
}

template<typename T>
void DeviceVector<T>::Flush()
{
    base.Flush();
}

template<typename T>
void DeviceVector<T>::Reset()
{
    base.Reset();
}
#endif // LETTUCE_CORE_DEVICE_VECTOR_INL