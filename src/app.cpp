#include "app.hpp"

#include <optional>
#include <span>
#include <string>
#include <thread>

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

#include "log.hpp"
#include "save.hpp"
#include "util.hpp"

namespace pewter {

struct Message {
    App *app;
    Save save;
};

struct AlertMessage {
    App *app;
    std::string message;
};

App::App() : Fl_Double_Window(340, 180, "Pewter") {
    menuBar = new Fl_Menu_Bar(0, 0, 340, 30);
    // The ampersand in front of the text makes the first letter a hotkey.
    menuBar->add("&File/&Open...", FL_CTRL + 'o', openFileCallback, this);

    playerContainer = new Fl_Flex(0, 30, 340, 150, Fl_Flex::VERTICAL);
    playerContainer->gap(5);
    playerContainer->margin(5);

    auto *playerNameRow = new Fl_Flex(Fl_Flex::HORIZONTAL);
    playerContainer->fixed(playerNameRow, 30);

    auto *playerNameLabel = new Fl_Box(0, 0, 0, 0, "Name:");
    playerNameLabel->align(FL_ALIGN_INSIDE);

    playerNameInput = new Fl_Input(0, 0, 0, 0);
    playerNameInput->value("");
    playerNameInput->maximum_size(7);

    playerNameRow->end();

    auto *playerGenderRow = new Fl_Flex(Fl_Flex::HORIZONTAL);
    playerContainer->fixed(playerGenderRow, 30);

    auto *playerGenderLabel = new Fl_Box(0, 0, 0, 0, "Gender:");
    playerGenderLabel->align(FL_ALIGN_INSIDE);

    boyButton = new Fl_Round_Button(0, 0, 0, 0, "Boy");
    boyButton->type(FL_RADIO_BUTTON);
    girlButton = new Fl_Round_Button(0, 0, 0, 0, "Girl");
    girlButton->type(FL_RADIO_BUTTON);

    playerGenderRow->end();

    playerContainer->end();

    playerContainer->hide();

    end();
}

void App::openFileCallback([[maybe_unused]] Fl_Widget *w, void *data) {
    // TODO: Show a confirmation prompt if there's already a save loaded.
    Fl_Native_File_Chooser fileChooser(Fl_Native_File_Chooser::BROWSE_FILE);
    fileChooser.filter("*.sav");
    if (auto rc = fileChooser.show()) {
        switch (rc) {
        case -1:
            fl_alert("An error occurred while trying to open a file.");
            log("failed to choose a file: {}", fileChooser.errmsg());
            break;
        case 1:
            return;
        default:
            log("unxpected return code: {}", rc);
            return;
        }
    }

    const auto *filename = fileChooser.filename();
    if (!filename) {
        log("unexpected NULL filename");
        return;
    }

    // Deactive the open file menu item to prevent the user from opening
    // another file while the current one is processed.
    auto *app = static_cast<App *>(data);
    // TODO: Is it safe to cast away const here? The FLTK docs do...
    auto *item =
        const_cast<Fl_Menu_Item *>(app->menuBar->find_item(openFileCallback));
    if (item) {
        item->deactivate();
    }

    std::thread thread(
        [](std::string filename, void *data) {
            log("opening file '{}'", filename);
            auto bytes = readFile(filename);
            if (!bytes) {
                log("could not open file '{}'", filename);

                auto *message = new AlertMessage();
                message->app = static_cast<App *>(data);
                message->message =
                    std::format("Could not open file '{}'.", filename);
                Fl::awake(show_alert_callback, static_cast<void *>(message));

                return;
            }

            auto save = parseSave(*bytes);
            if (!save) {
                return;
            }

            auto *message = new Message();
            message->app = static_cast<App *>(data);
            message->save = *save;
            Fl::awake(show_save_callback, static_cast<void *>(message));
        },
        std::string(filename), data);
    thread.detach();
}

void App::show_save_callback(void *data) {
    auto *message = static_cast<Message *>(data);
    auto *app = message->app;
    app->playerNameInput->value("ZACH");
    switch (message->save.gender) {
    case Gender::Boy:
        app->boyButton->setonly();
        break;
    case Gender::Girl:
        app->girlButton->setonly();
        break;
    case Gender::None:
        break;
    }
    app->playerContainer->show();

    auto *item =
        const_cast<Fl_Menu_Item *>(app->menuBar->find_item(openFileCallback));
    if (item) {
        item->activate();
    }

    delete message;
}

void App::show_alert_callback(void *data) {
    auto *message = static_cast<AlertMessage *>(data);
    fl_alert("%s", message->message.c_str());
    delete message;
}

}
