#pragma once

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Widget.H>

namespace pewter {

// TODO: Child widgets get cleaned up by Fl_Double_Window's destructor. Check
// if anything needs to be cleaned up manually.
class App final : public Fl_Double_Window {
  public:
    App();

  private:
    // TODO: Use Fl_Sys_Menu_Bar to get the native menu bar on macOS.
    Fl_Menu_Bar *menu_bar;
    Fl_Flex *flex;
    Fl_Input *player_name_input;

    static void open_file_callback(Fl_Widget *w, void *user_data);
};

}
