#pragma once

#include <cstdint>
#include <optional>
#include <span>

namespace pewter
{

enum class Gender
{
    Boy,
    Girl,
    None,
};

struct Save
{
    Gender gender;
};

/// Parses game save information from `bytes`.
std::optional<Save> parseSave(std::span<const std::uint8_t> bytes);

} // namespace pewter
