#include <bits/stdint-uintn.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libpewter/save.h>

#define SAVE_SIZE 131072
#define BLOCK_SIZE 57344
#define SECTION_SIZE 4096

#define SECTION_ID_OFFSET 0x0FF4
#define CHECKSUM_OFFSET 0x0FF6
#define SAVE_INDEX_OFFSET 0x0FFC

#define TRAINER_INFO_SECTION_ID 0

#define PLAYER_GENDER_OFFSET 0x0008

#define PLAYER_NAME_MAX_LEN 7

uint16_t read_uint16_t(const uint8_t *buf) {
    return buf[0] | buf[1] << 8;
}

uint32_t read_uint32_t(const uint8_t *buf) {
    return buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
}

struct pewter_save {
    uint8_t *buf;
    size_t block_offset;
    size_t section_offsets[14];

    char trainer_name[7];
};

// '&' is a sentinel value that means the character isn't ASCII or can't be
// used in names.
static char char_set[] = {
    '0', '1', '2', '3', '4', '5',  '6',  '7', '8', '9', '!', '?', '.',
    '-', '&', '&', '"', '"', '\'', '\'', '&', '&', '&', ',', '&', '/',
    'A', 'B', 'C', 'D', 'E', 'F',  'G',  'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S',  'T',  'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f',  'g',  'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's',  't',  'u', 'v', 'w', 'x', 'y', 'z'};

static uint16_t section_checksum_bytes[] = {3884, 3968, 3968, 3968, 3848,
                                            3968, 3968, 3968, 3968, 3968,
                                            3968, 3968, 3968, 2000};

/// Decodes byte `b` into character `c`.
static int decode(const unsigned char b, char *c) {
    if (b < 0xA1 || b > 0xEE) {
        return -1;
    }

    char tmp = char_set[b - 0xA1];
    if (tmp == '&') {
        return -1;
    }

    *c = tmp;

    return 0;
}

/// data is a pointer to a section data part.
uint16_t section_checksum(const uint8_t *data, uint16_t size) {
    // TODO: Assert (debug?) that size is divisible by 4;
    uint32_t sum = 0;
    for (int i = 0; i < size; i += 4) {
        sum += read_uint32_t(data + i);
    }
    return (uint16_t)sum + (uint16_t)(sum >> 8);
}

int validate_block(const uint8_t *blk) {
    uint32_t save_index = read_uint32_t(blk + SAVE_INDEX_OFFSET);

    for (int i = 0; i < 14; ++i) {
        const uint8_t *sec = blk + i * SECTION_SIZE;

        if (save_index != read_uint32_t(sec + SAVE_INDEX_OFFSET)) {
            return -1;
        }

        // TODO: Check signature.

        uint16_t id = read_uint16_t(sec + SECTION_ID_OFFSET);

        uint16_t checksum = section_checksum(sec, section_checksum_bytes[id]);
        if (checksum != read_uint16_t(sec + CHECKSUM_OFFSET)) {
            fprintf(stderr, "error: invalid checksum for section %d\n", id);
            return -1;
        }
        fprintf(stderr, "cool: valid checksum for section %d\n", id);
    }

    return 0;
}

struct pewter_save *pewter_parse(const uint8_t *buf, size_t size) {
    if (!buf || size < SAVE_SIZE) {
        return NULL;
    }

    const uint8_t *block_a = buf;
    const uint8_t *block_b = buf + BLOCK_SIZE;

    // Cheat for now and just read the save index from the first section in
    // each block.
    uint32_t save_index_a = read_uint32_t(block_a + SAVE_INDEX_OFFSET);
    uint32_t save_index_b = read_uint32_t(block_b + SAVE_INDEX_OFFSET);

    if (validate_block(block_a) || validate_block(block_b)) {
        return NULL;
    }

    size_t block_offset = save_index_a < save_index_b ? BLOCK_SIZE : 0;

    size_t section_offsets[14];
    for (int section_offset = block_offset;
         section_offset < block_offset + BLOCK_SIZE;
         section_offset += SECTION_SIZE) {
        uint16_t section_id =
            read_uint16_t(buf + section_offset + SECTION_ID_OFFSET);
        section_offsets[section_id] = section_offset;
    }

    void *mem = malloc(sizeof(struct pewter_save) + size);
    if (!mem) {
        return NULL;
    }

    struct pewter_save *save = (struct pewter_save *)mem;
    save->buf = mem + sizeof(struct pewter_save);
    memcpy(save->buf, buf, size);
    save->block_offset = block_offset;
    memcpy(save->section_offsets, &section_offsets, sizeof(section_offsets));

    return save;
}

int pewter_save_player_name(const struct pewter_save *save, char *name,
                            size_t len) {
    if (!name || len < (PLAYER_NAME_MAX_LEN + 1)) {
        return -1;
    }

    size_t section_offset = save->section_offsets[TRAINER_INFO_SECTION_ID];

    int i = 0;
    // TODO: Rename b to decoded string or something.
    for (uint8_t *b = save->buf + section_offset + i;
         i < PLAYER_NAME_MAX_LEN && *b != 0xFF; ++i, ++b) {
        if (decode(*b, &name[i])) {
            return -1;
        }
    }

    name[i] = '\0';

    return 0;
}

int pewter_save_player_gender(const struct pewter_save *save,
                              enum pewter_gender *gender) {
    int rc = 0;

    size_t section_offset = save->section_offsets[TRAINER_INFO_SECTION_ID];
    uint8_t data = save->buf[section_offset + PLAYER_GENDER_OFFSET];
    switch (data) {
    case 0:
        *gender = pewter_gender_boy;
        break;
    case 1:
        *gender = pewter_gender_girl;
        break;
    default:
        rc = -1;
    }

    return rc;
};
