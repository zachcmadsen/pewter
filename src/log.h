#include <fmt/core.h>

template <typename... T> void log(fmt::format_string<T...> fmt, T &&...args) {
    auto msg = fmt::format(fmt, std::forward<T>(args)...);
    fmt::print(stderr, "{}\n", msg);
}
