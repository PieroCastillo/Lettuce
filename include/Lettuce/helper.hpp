/*
Created by @PieroCastillo on 2025-01-9
*/
#ifndef LETTUCE_HELPER_HPP
#define LETTUCE_HELPER_HPP

#include <print>

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

#endif // LETTUCE_HELPER_HPP