#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <span>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <vector>

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Round_Button.H>
#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>
#include <FL/platform_types.h>

#include "app.hpp"
#include "log.hpp"
#include "util.hpp"

namespace pewter {

/// The size of a section in bytes.
inline constexpr size_t section_size = 4096;
/// The number of sections in a block.
inline constexpr size_t sections = 14;

/// Reads a 8-bit unsigned integer from `bytes`.
uint16_t read_u8(std::span<const uint8_t, 1> bytes) {
    return bytes[0];
}

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
    for (size_t i = 0; i < sections; ++i) {
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

Gender read_gender(std::span<const uint8_t, section_size> section) {
    constexpr size_t gender_offset = 0x0008;

    Gender gender;
    switch (section[gender_offset]) {
    case 0x00:
        gender = Gender::Boy;
        break;
    case 0x01:
        gender = Gender::Girl;
        break;
    default:
        log("invalid player gender: {}", section[gender_offset]);
        gender = Gender::None;
        break;
    }

    return gender;
}

struct Message {
    App *app;
    Save save;
};

App::App() : Fl_Double_Window(340, 180, "Pewter") {
    menu_bar = new Fl_Menu_Bar(0, 0, 340, 30);
    // The ampersand in front of the text makes the first letter a hotkey.
    menu_bar->add("&File/&Open...", FL_CTRL + 'o', open_file_callback, this);

    player_container = new Fl_Flex(0, 30, 340, 150, Fl_Flex::VERTICAL);
    player_container->margin(5);

    Fl_Flex *player_name_row = new Fl_Flex(Fl_Flex::HORIZONTAL);

    // It doesn't matter what we put the size and position of the widgets
    // since they'll be resized anyways.
    Fl_Box *player_name_label = new Fl_Box(0, 0, 0, 0, "Name:");
    player_name_label->align(FL_ALIGN_INSIDE);

    // TODO: Investigate how measure_label works.
    int w, h;
    player_name_label->measure_label(w, h);
    player_name_row->fixed(player_name_label, w + 5);

    player_name_input = new Fl_Input(0, 0, 0, 0);
    player_name_input->value("");
    player_name_input->maximum_size(7);

    player_name_row->end();
    player_container->fixed(player_name_row, 30);

    Fl_Flex *player_gender_row = new Fl_Flex(Fl_Flex::HORIZONTAL);
    Fl_Box *player_gender_label = new Fl_Box(0, 0, 0, 0, "Gender:");
    player_gender_label->align(FL_ALIGN_INSIDE);

    player_gender_label->measure_label(w, h);
    player_gender_row->fixed(player_gender_label, w + 5);

    boy_radio_button = new Fl_Round_Button(0, 0, 0, 0, "Boy");
    boy_radio_button->type(FL_RADIO_BUTTON);
    girl_radio_button = new Fl_Round_Button(0, 0, 0, 0, "Girl");
    girl_radio_button->type(FL_RADIO_BUTTON);
    player_gender_row->end();
    player_container->fixed(player_gender_row, 30);

    player_container->gap(5);

    player_container->hide();
    player_container->end();

    end();
}

void App::open_file_callback(Fl_Widget *, void *data) {
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

    // Prevent the user from opening another file while the current one is
    // processed.
    App *app = static_cast<App *>(data);
    // TODO: Is it safe to cast away const here? The FLTK docs do...
    Fl_Menu_Item *item = const_cast<Fl_Menu_Item *>(
        app->menu_bar->find_item(open_file_callback));
    if (item) {
        item->deactivate();
    }

    std::thread thread(
        [](std::string filename, void *x) {
            auto save = read_file(filename);
            if (!save) {
                return;
            }

            auto block = std::span<uint8_t, 57344>((*save).data(), 57344);
            validate_block(block);

            Message *message = new Message();
            message->app = static_cast<App *>(x);

            for (size_t i = 0; i < sections; ++i) {
                auto section =
                    block.subspan(i * section_size).first<section_size>();
                auto section_id = read_section_id(section);

                switch (section_id) {
                case 0: // trainer info
                    message->save.gender = read_gender(section);
                    break;
                };
            }

            Fl::awake(show_save_callback, static_cast<void *>(message));
        },
        std::string(filename), data);
    thread.detach();
}

void App::show_save_callback(void *data) {
    auto message = static_cast<Message *>(data);
    auto app = message->app;
    app->player_name_input->value("ZACH");
    switch (message->save.gender) {
    case Gender::Boy:
        app->boy_radio_button->setonly();
        break;
    case Gender::Girl:
        app->girl_radio_button->setonly();
        break;
    case Gender::None:
        break;
    }
    app->player_container->show();

    Fl_Menu_Item *item = const_cast<Fl_Menu_Item *>(
        app->menu_bar->find_item(open_file_callback));
    if (item) {
        item->activate();
    }

    delete message;
}

}
