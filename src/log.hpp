#pragma once

#include <cstdio>
#include <format>
#include <print>

namespace pewter {

/// Logs a message to `stderr`.
template <typename... T> void log(std::format_string<T...> fmt, T &&...args) {
    auto msg = std::format(fmt, std::forward<T>(args)...);
    std::print(stderr, "{}\n", msg);
}

}
