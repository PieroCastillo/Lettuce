/*
Created by @PieroCastillo on 2025-01-9
*/
#ifndef LETTUCE_HELPER_HPP
#define LETTUCE_HELPER_HPP

#ifndef NDEBUG

#include <cstdint>
#include <iostream>
#include <print>
#include <stacktrace>
#include <string>

    #define DEBUG_YELLOW_BOLD "\033[1;33m"
    #define DEBUG_RESET       "\033[0m"

    #define DebugPrint(title, content, ...)                      \
        std::println(                                            \
            DEBUG_YELLOW_BOLD "{}" DEBUG_RESET " " content,      \
            title, ##__VA_ARGS__                                 \
        )

    #define DebugAssert(cond, msg)                                \
    do {                                                          \
        if (!(cond)) [[unlikely]] {                               \
            std::println(std::cerr,                               \
                "\033[1;31m[ASSERTION FAILED]\033[0m");             \
            std::println(std::cerr, "Condition: {}", #cond);      \
            std::println(std::cerr, "Message: {}", msg);          \
            std::println(std::cerr, "{}",                         \
                std::stacktrace::current());                      \
            std::abort();                                         \
        }                                                         \
    } while(false)

#else
    #define DebugPrint(...)  ((void)0)
    #define DebugAssert(...) ((void)0)
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