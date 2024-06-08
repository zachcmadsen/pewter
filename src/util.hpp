#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace pewter {

/// Reads a 16-bit unsigned integer from `bytes`.
inline std::uint16_t readU16(std::span<const std::uint8_t, 2> bytes) {
    return static_cast<std::uint16_t>(bytes[0] | bytes[1] << 8);
}

/// Reads a 32-bit unsigned integer from `bytes`.
inline std::uint32_t readU32(std::span<const std::uint8_t, 4> bytes) {
    return static_cast<std::uint32_t>(bytes[0] | bytes[1] << 8 |
                                      bytes[2] << 16 | bytes[3] << 24);
}

/// Reads the contents of a file into a vector of bytes.
std::optional<std::vector<std::uint8_t>> readFile(std::string_view filename);

}
