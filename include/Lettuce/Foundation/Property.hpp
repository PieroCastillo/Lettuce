//
// Created by piero on 19/12/2024.
//
#pragma once

namespace Lettuce::Foundation
{
    template<typename T>
    class Property<T>
    {
        private:
        T element;
        public:
        T& get();
        void set(T element);
    };
}