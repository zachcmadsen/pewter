#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace pewter {

/// Reads the contents of a file into a vector of bytes.
std::optional<std::vector<uint8_t>> read_file(const char *filename);

}
