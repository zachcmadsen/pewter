#include <cstdint>
#include <cstdio>
#include <memory>
#include <optional>
#include <vector>

#include "FL/fl_utf8.h"

#include "log.hpp"

namespace pewter {

std::optional<std::vector<uint8_t>> read_file(const char *filename) {
    // TODO: Use C++ file I/O instead of C file I/O.
    log("opening file '{}'", filename);
    std::unique_ptr<FILE, decltype(std::fclose) *> fp(fl_fopen(filename, "rb"),
                                                      std::fclose);
    if (!fp) {
        log("could not open file '{}'", filename);
        return {};
    }

    if (std::fseek(fp.get(), 0, SEEK_END) != 0) {
        log("fseek failed");
        return {};
    }

    long file_size = std::ftell(fp.get());
    if (file_size == -1) {
        log("ftell failed");
        return {};
    }

    if (std::fseek(fp.get(), 0, SEEK_SET) != 0) {
        log("fseek failed");
        return {};
    }

    // TODO: If file_size is greater than the size of a save, truncate it?
    std::vector<uint8_t> bytes(file_size);
    std::size_t bytes_read =
        std::fread(bytes.data(), sizeof(uint8_t), file_size, fp.get());
    if (bytes_read != static_cast<std::size_t>(file_size)) {
        log("failed to read file");
        return {};
    }

    return bytes;
}

}
