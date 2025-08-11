/*
Created by @PieroCastillo on 2025-07-28
*/
#ifndef LETTUCE_CORE_RAGGED_ARRAY_HPP
#define LETTUCE_CORE_RAGGED_ARRAY_HPP

#include <vector>
#include <span>

template<typename T>
struct RaggedArray {
    std::vector<T> data;
    std::vector<size_t> offsets;
    
    void build(const std::vector<std::vector<T>>& lists) {
        offsets.reserve(lists.size());
        size_t current = 0;
        for (auto& l : lists) {
            offsets.push_back(current);
            data.insert(data.end(), l.begin(), l.end());
            current += l.size();
        }
    }

    std::span<const T> get(size_t i) const {
        size_t start = offsets[i];
        size_t end   = (i + 1 < offsets.size()) ? offsets[i + 1] : data.size();
        return { data.data() + start, end - start };
    }
};
#endif // LETTUCE_CORE_RAGGED_ARRAY_HPP