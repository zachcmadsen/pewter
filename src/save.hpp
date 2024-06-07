#pragma once

#include <cstdint>
#include <optional>
#include <span>

namespace pewter {

enum class Gender {
    None,
    Boy,
    Girl,
};

struct Save {
    Gender gender;
};

std::optional<Save> parseSave(std::span<const std::uint8_t> bytes);

}
