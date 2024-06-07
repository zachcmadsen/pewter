#include <cstdint>
#include <fstream>
#include <ios>
#include <iterator>
#include <optional>
#include <string_view>
#include <vector>

#include "util.hpp"

namespace pewter {

std::optional<std::vector<uint8_t>> read_file(std::string_view filename) {
    std::ifstream ifs(filename.data(), std::ios::binary);
    if (!ifs.is_open()) {
        return {};
    }

    std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(ifs)),
                                    std::istreambuf_iterator<char>());

    return bytes;
}

}
