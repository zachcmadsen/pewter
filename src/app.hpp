#pragma once

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Widget.H>

namespace pewter {

enum class Gender {
    None,
    Boy,
    Girl,
};

struct Save {
    Gender gender;
};

// TODO: Child widgets get cleaned up by Fl_Double_Window's destructor. Check
// if anything needs to be cleaned up manually.
class App final : public Fl_Double_Window {
  public:
    App();

  private:
    // TODO: Use Fl_Sys_Menu_Bar to get the native menu bar on macOS.
    Fl_Menu_Bar *menu_bar;
    Fl_Flex *player_container;
    Fl_Input *player_name_input;
    Fl_Round_Button *boy_radio_button;
    Fl_Round_Button *girl_radio_button;

    static void open_file_callback(Fl_Widget *w, void *data);
    static void show_save_callback(void *data);
};

}
