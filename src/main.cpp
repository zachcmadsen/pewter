// TODO: Turn the include header diagnostics back on.
#include "FL/Enumerations.H"
#include "FL/Fl_Double_Window.H"
#include "FL/Fl_Input.H"
#include "FL/fl_ask.H"
#include "FL/fl_utf8.h"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

// extern "C" {
// #include <libpewter/save.h>
// }

class App : public Fl_Double_Window {
  public:
    App();

  private:
    // Note, child widgets are cleanup by Fl_Double_Window's destructor.
    // So... I don't need to have fields for widgets that I don't interact
    // with after their construction??
    // TODO: Clean up the other stuff though...
    Fl_Menu_Bar *menu_bar;
    Fl_Box *player_name_label;
    Fl_Input *player_name_input;
    Fl_Flex *flex;

    std::vector<uint8_t> save;

    static void open_file_callback(Fl_Widget *w, void *userdata);
};

App::App() : Fl_Double_Window(340, 180, "Pewter") {
    // TODO: Use sys menu item, and add a comment explaining why.
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
    player_name_label = new Fl_Box(0, 0, 0, 0, "Player name:");
    // TODO: Investigate how measure_label works.
    int w, h;
    player_name_label->measure_label(w, h);
    player_name_label->align(FL_ALIGN_INSIDE);
    player_name_row->fixed(player_name_label, w + 5);

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

void try_read_file(std::string filename) {
    std::cout << "selected file: " << filename.c_str() << '\n';

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2000ms);
    FILE *fp = fl_fopen(filename.c_str(), "rb");

    struct stat stat;
    if (fl_stat(filename.c_str(), &stat)) {
        fl_alert("Failed to process file '%s'", filename.c_str());
        return;
    }

    std::cout << "file size: " << stat.st_size << '\n';

    // TODO: Use fmt::print to log everything to stderr. Add an internal
    // logging
    // system later.

    std::vector<uint8_t> save(stat.st_size);
    auto bytes_read = fread(save.data(), sizeof(uint8_t), stat.st_size, fp);

    if (bytes_read == stat.st_size) {
        std::cout << "successfully read file!" << '\n';
    }

    // delete[] filename;
}

void App::open_file_callback(Fl_Widget *w, void *userdata) {
    App *app = static_cast<App *>(userdata);

    // TODO: Reset stuff if there was already a sav loaded? Also display
    // a confirmation prompt if they would lose work?

    std::cout << "open callback\n";

    Fl_Native_File_Chooser file_chooser(Fl_Native_File_Chooser::BROWSE_FILE);
    file_chooser.filter("*.sav");

    if (file_chooser.show()) {
        // TODO: Handle error and cancel cases.
        return;
    }

    const char *filename = file_chooser.filename();
    if (!filename) {
        return;
    }

    // char *filename_copy = new char[strlen(filename) + 1];
    // strcpy(filename_copy, filename);
    std::thread thread(try_read_file, std::string(filename));
    thread.detach();

    // TODO: Show loading indicator or something.

    // std::cout << "selected file: " << filename << '\n';
    // FILE *fp = fl_fopen(filename, "rb");

    // struct stat stat;
    // if (fl_stat(filename, &stat)) {
    //     fl_alert("Failed to process file '%s'", filename);
    //     return;
    // }

    // std::cout << "file size: " << stat.st_size << '\n';

    // // TODO: Use fmt::print to log everything to stderr. Add an internal
    // logging
    // // system later.

    // app->save = std::vector<uint8_t>(stat.st_size);
    // auto bytes_read =
    //     fread(app->save.data(), sizeof(uint8_t), stat.st_size, fp);

    // if (bytes_read == stat.st_size) {
    //     std::cout << "successfully read file!" << '\n';
    // }

    // app->player_name_input->value("Zach");
    // app->flex->show();
}

int main() {
    App *app = new App();
    app->show();

    return Fl::run();
}
