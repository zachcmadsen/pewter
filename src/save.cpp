#include "save.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include "log.hpp"
#include "util.hpp"

namespace pewter {

/// The size of a block in bytes.
inline constexpr std::size_t BlockSize = 57344;
/// The size of a section in bytes.
inline constexpr std::size_t SectionSize = 4096;

/// The number of sections in a block.
inline constexpr std::size_t sections = 14;

inline constexpr std::uint16_t TrainerInfoSectionId = 0;

inline constexpr std::size_t SectionIdOffset = 0x0FF4;
inline constexpr std::size_t SectionChecksumOffset = 0x0FF6;
inline constexpr std::size_t SaveIndexOffset = 0x0FFC;

inline constexpr std::size_t PlayerGenderOffset = 0x0008;

inline constexpr std::array<std::uint16_t, sections> ChecksumBytes{
    3884, 3968, 3968, 3968, 3948, 3968, 3968,
    3968, 3968, 3968, 3968, 3968, 3968, 2000};

std::uint16_t
readSectionId(std::span<const std::uint8_t, SectionSize> section) {
    return readU16(section.subspan<SectionIdOffset, 2>());
}

std::uint16_t readChecksum(std::span<const std::uint8_t, SectionSize> section) {
    return readU16(section.subspan<SectionChecksumOffset, 2>());
}

std::uint32_t
readSaveIndex(std::span<const std::uint8_t, SectionSize> section) {
    return readU32(section.subspan<SaveIndexOffset, 4>());
}

std::uint16_t
computeChecksum(std::span<const std::uint8_t, SectionSize> section) {
    auto sectionId = readSectionId(section);
    auto bytes = ChecksumBytes[sectionId];
    std::uint32_t checksum = 0;
    for (int i = 0; i < bytes; i += 4) {
        checksum += readU32(section.subspan(i).first<4>());
    }

    return static_cast<std::uint16_t>(checksum) +
           static_cast<std::uint16_t>(checksum >> 16);
}

void validateBlock(std::span<const std::uint8_t, BlockSize> block) {
    auto firstSection = block.first<SectionSize>();
    auto firstSaveIndex = readSaveIndex(firstSection);

    // TODO: Validate the section signature and that each section appears only
    // once. I could also validate that the sections are in order, but that's
    // probably not necessary.
    for (size_t i = 0; i < sections; ++i) {
        auto section = block.subspan(i * SectionSize).first<SectionSize>();

        auto checksum = computeChecksum(section);
        auto actual_checksum = readChecksum(section);
        if (actual_checksum != checksum) {
            log("checksum mismatch for section: {} != {}", actual_checksum,
                checksum);
        }

        auto save_index = readSaveIndex(section);
        if (save_index != firstSaveIndex) {
            log("save index mismatch: {} != {}", firstSaveIndex, save_index);
        }
    }

    log("block is valid");
}

Gender readGender(std::span<const std::uint8_t, SectionSize> section) {
    Gender gender;
    switch (section[PlayerGenderOffset]) {
    case 0x00:
        gender = Gender::Boy;
        break;
    case 0x01:
        gender = Gender::Girl;
        break;
    default:
        log("invalid player gender: {}", section[PlayerGenderOffset]);
        gender = Gender::None;
        break;
    }

    return gender;
}

std::optional<Save> parseSave(std::span<const std::uint8_t> bytes) {
    if (bytes.size() < BlockSize) {
        return {};
    }

    auto block = bytes.first<BlockSize>();
    validateBlock(block);

    Save save;
    for (size_t i = 0; i < sections; ++i) {
        auto section = block.subspan(i * SectionSize).first<SectionSize>();
        auto sectionId = readSectionId(section);

        switch (sectionId) {
        case TrainerInfoSectionId:
            save.gender = readGender(section);
            break;
        };
    }

    return save;
}

}
