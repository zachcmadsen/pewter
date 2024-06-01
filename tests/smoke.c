#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libpewter/save.h>

void err(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "error: ");

    vfprintf(stderr, fmt, args);
    va_end(args);
}

int read_file(const char *filename, uint8_t **buf, size_t *size) {
    int rc = 0;
    uint8_t *file_buf = NULL;

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        rc = -1;
        goto error;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        rc = -1;
        goto error;
    }

    // ftell returns the number of bytes from the beginning of the file to the
    // stream cursor if the stream is opened in binary mode.
    long file_size = ftell(fp);
    if (file_size == -1L) {
        rc = -1;
        goto error;
    }

    // Go back to the beginning before reading.
    if (fseek(fp, 0, SEEK_SET) != 0) {
        rc = -1;
        goto error;
    }

    file_buf = malloc(file_size);
    if (!file_buf) {
        rc = -1;
        goto error;
    }

    if (fread(file_buf, 1, file_size, fp) != (unsigned long)file_size) {
        rc = -1;
        goto error;
    }

    *buf = file_buf;
    *size = file_size;

cleanup:
    if (fp) {
        fclose(fp);
    }

    return rc;

error:
    if (file_buf) {
        free(file_buf);
    }

    goto cleanup;
}

int smoke(const char *filename) {
    int rc = 0;
    uint8_t *save_data = NULL;
    struct pewter_save *save = NULL;

    size_t save_data_size;
    if (read_file(filename, &save_data, &save_data_size)) {
        err("could not read file '%s'\n", filename);
        rc = -1;
        goto cleanup;
    }

    save = pewter_parse(save_data, save_data_size);
    if (!save) {
        err("invalid save\n");
        rc = -1;
        goto cleanup;
    }

    enum pewter_gender gender;
    if (pewter_save_player_gender(save, &gender)) {
        err("invalid trainer gender value\n");
        rc = -1;
        goto cleanup;
    };
    if (gender != pewter_gender_girl) {
        err("expected trainer gender to be girl, got %d\n", gender);
        rc = -1;
        goto cleanup;
    }

    char trainer_name[8];
    if (pewter_save_player_name(save, trainer_name, sizeof(trainer_name))) {
        rc = -1;
        goto cleanup;
    }
    if (strcmp(trainer_name, "ZACH")) {
        err("expected trainer name to be ZACH, got %s\n", trainer_name);
        rc = -1;
        goto cleanup;
    }

cleanup:
    if (save) {
        free(save);
    }

    if (save_data) {
        free(save_data);
    }

    return rc;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        err("no input file\n");
        return EXIT_FAILURE;
    }

    char *filename = argv[1];
    if (smoke(filename)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}