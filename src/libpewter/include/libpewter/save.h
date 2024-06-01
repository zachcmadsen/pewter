#pragma once

#include <stddef.h>
#include <stdint.h>

struct pewter_save;

enum pewter_gender {
    pewter_gender_boy,
    pewter_gender_girl,
};

struct pewter_save *pewter_parse(const uint8_t *data, size_t size);

int pewter_save_player_name(const struct pewter_save *save, char *name,
                            size_t len);

int pewter_save_player_gender(const struct pewter_save *save,
                              enum pewter_gender *gender);
