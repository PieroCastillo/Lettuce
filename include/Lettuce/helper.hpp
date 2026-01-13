/*
Created by @PieroCastillo on 2025-01-9
*/
#ifndef LETTUCE_HELPER_HPP
#define LETTUCE_HELPER_HPP

#include <print>
#include <cstdint>
#include <string>

#ifndef NDEBUG

    #define DEBUG_YELLOW_BOLD "\033[1;33m"
    #define DEBUG_RESET       "\033[0m"

    #define DebugPrint(title, content, ...)                          \
        std::println(                                                \
            DEBUG_YELLOW_BOLD "{}" DEBUG_RESET " " content,         \
            title, ##__VA_ARGS__                                     \
        )

#else

    #define DebugPrint(...) ((void)0)

#endif


inline std::string hexData(const void* ptr, std::size_t size)
{
    auto bytes = static_cast<const std::uint8_t*>(ptr);
    std::string out;
    out.reserve(size * 3);

    for (std::size_t i = 0; i < size; ++i)
    {
        out += std::format("{:02X}", bytes[i]);
        if (i + 1 < size)
            out += ' ';
    }
    return out;
}

#endif // LETTUCE_HELPER_HPP