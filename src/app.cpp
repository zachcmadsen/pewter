#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <vector>

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>
#include <FL/platform_types.h>

#include "app.h"
#include "log.h"

namespace pewter {

/// The size of a section in bytes.
inline constexpr size_t section_size = 4096;
/// The number of sections in a block.
inline constexpr size_t sections = 14;

/// Reads a 16-bit unsigned integer from `bytes`.
uint16_t read_u16(std::span<const uint8_t, 2> bytes) {
    return bytes[0] | bytes[1] << 8;
}

/// Reads a 32-bit unsigned integer from `bytes`.
uint32_t read_u32(std::span<const uint8_t, 4> bytes) {
    return bytes[0] | bytes[1] << 8 | bytes[2] << 16 | bytes[3] << 24;
}

/// Reads the section ID from `section`.
uint16_t read_section_id(std::span<const uint8_t, section_size> section) {
    constexpr size_t section_id_offset = 0x0FF4;
    return read_u16(section.subspan<section_id_offset, 2>());
}

/// Reads the section checksum from `section`.
uint16_t read_checksum(std::span<const uint8_t, section_size> section) {
    constexpr size_t checksum_offset = 0x0FF6;
    return read_u16(section.subspan<checksum_offset, 2>());
}

/// Reads the save index from `section`.
uint32_t read_save_index(std::span<const uint8_t, section_size> section) {
    constexpr size_t save_index_offset = 0x0FFC;
    return read_u32(section.subspan<save_index_offset, 4>());
}

/// Computes the checksum of a section.
uint16_t
compute_section_checksum(std::span<const uint8_t, section_size> section) {
    constexpr std::array<uint16_t, sections> checksum_bytes_per_section{
        3884, 3968, 3968, 3968, 3948, 3968, 3968,
        3968, 3968, 3968, 3968, 3968, 3968, 2000};

    auto section_id = read_section_id(section);
    auto bytes = checksum_bytes_per_section[section_id];
    uint32_t checksum = 0;
    for (int i = 0; i < bytes; i += 4) {
        checksum += read_u32(section.subspan(i).first<4>());
    }

    return static_cast<uint16_t>(checksum) +
           static_cast<uint16_t>(checksum >> 16);
}

/// Validates a game save `block`.
void validate_block(std::span<const uint8_t, 57344> block) {
    auto first_section = block.first<section_size>();
    auto first_save_index = read_save_index(first_section);

    // TODO: Validate the section signature and that each section appears only
    // once. I could also validate that the sections are in order, but that's
    // probably not necessary.
    for (int i = 0; i < sections; ++i) {
        auto section = block.subspan(i * section_size).first<section_size>();

        auto checksum = compute_section_checksum(section);
        auto actual_checksum = read_checksum(section);
        if (actual_checksum != checksum) {
            log("checksum mismatch for section: {} != {}", actual_checksum,
                checksum);
        }

        auto save_index = read_save_index(section);
        if (save_index != first_save_index) {
            log("save index mismatch: {} != {}", first_save_index, save_index);
        }
    }

    log("block is valid");
}

/// Parses a save file and updates the GUI thread with the results.
void parse_save(void *app, std::string filename) {
    log("parsing file '{}'", filename);

    struct stat stat;
    if (fl_stat(filename.c_str(), &stat)) {
        log("fl_state failed");
        return;
    }

    if (stat.st_size < 131072) {
        log("invalid sav file size: {}", stat.st_size);
        return;
    }

    FILE *fp = fl_fopen(filename.c_str(), "rb");
    if (!fp) {
        log("could not open file '{}'", filename);
        return;
    }

    // TODO: Don't read in more than the save size.
    std::vector<uint8_t> save(stat.st_size);
    auto read = fread(save.data(), sizeof(uint8_t), stat.st_size, fp);
    fclose(fp);
    if (read != stat.st_size) {
        log("failed to read file '{}'", filename);
        return;
    }

    validate_block(std::span<uint8_t, 57344>(save.data(), 57344));

    // Message *msg = new Message();
    // msg->app = static_cast<App *>(app);
    // msg->file_size = stat.st_size;
    // Fl::awake(show_file_size_callback, (void *)msg);
}

struct Message {
    App *app;
    int file_size;
};

App::App() : Fl_Double_Window(340, 180, "Pewter") {
    // The ampersand in front of the text makes the first letter a hotkey.
    Fl_Menu_Item menu_items[] = {
        {"&File", 0, 0, 0, FL_SUBMENU},
        {"&Open...", FL_COMMAND + 'o', open_file_callback, this},
        {0},
        {0}};
    menu_bar = new Fl_Menu_Bar(0, 0, 340, 30);
    menu_bar->copy(menu_items);

    flex = new Fl_Flex(0, 30, 340, 150, Fl_Flex::VERTICAL);
    flex->margin(5);

    Fl_Flex *player_name_row = new Fl_Flex(Fl_Flex::HORIZONTAL);

    // It doesn't matter what we put the size and position of the widgets
    // since they'll be resized anyways.
    Fl_Box *player_name_label = new Fl_Box(0, 0, 0, 0, "Player name:");
    // TODO: Investigate how measure_label works.
    int w, h;
    player_name_label->measure_label(w, h);
    player_name_label->align(FL_ALIGN_INSIDE);
    player_name_row->fixed(player_name_label, w);

    player_name_input = new Fl_Input(0, 0, 0, 0);
    // player_name_input->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
    player_name_input->value("");
    player_name_input->maximum_size(7);

    player_name_row->end();
    flex->fixed(player_name_row, 30);

    // flex->hide();
    flex->end();

    end();
}

void show_file_size_callback(void *user_data) {
    Message *msg = static_cast<Message *>(user_data);
    fl_alert("the size of the file is %d", msg->file_size);
    delete msg;
}

void App::open_file_callback(Fl_Widget *w, void *app) {
    // TODO: Show a confirmation prompt if there's already a save loaded.
    Fl_Native_File_Chooser file_chooser(Fl_Native_File_Chooser::BROWSE_FILE);
    file_chooser.filter("*.sav");
    if (int rc = file_chooser.show()) {
        switch (rc) {
        case -1:
            fl_alert("An error occurred while trying to open a file.");
            log("failed to choose a file: {}", file_chooser.errmsg());
            break;
        case 1:
            return;
        default:
            log("unxpected return code: {}", rc);
            return;
        }
    }

    const char *filename = file_chooser.filename();
    if (!filename) {
        log("unexpected NULL filename");
        return;
    }

    std::thread thread(parse_save, app, std::string(filename));
    thread.detach();
}

}
