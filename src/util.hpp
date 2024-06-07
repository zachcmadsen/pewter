#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace pewter {

/// Reads the contents of a file into a vector of bytes.
std::optional<std::vector<uint8_t>> readFile(std::string_view filename);

}
