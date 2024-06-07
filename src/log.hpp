#pragma once

#include <cstdio>

#include <fmt/base.h>
#include <fmt/core.h>

namespace pewter {

/// Logs a message to `stderr`.
template <typename... T> void log(fmt::format_string<T...> fmt, T &&...args) {
    auto msg = fmt::format(fmt, std::forward<T>(args)...);
    fmt::print(stderr, "{}\n", msg);
}

}
