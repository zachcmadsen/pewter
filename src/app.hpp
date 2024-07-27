#pragma once

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Widget.H>

namespace pewter
{

class App final : public Fl_Double_Window
{
  public:
    App();

  private:
    // TODO: Use Fl_Sys_Menu_Bar to get the native menu bar on macOS.
    Fl_Menu_Bar *menuBar;
    Fl_Flex *playerContainer;
    Fl_Input *playerNameInput;
    Fl_Round_Button *boyButton;
    Fl_Round_Button *girlButton;

    static void openFileCallback(Fl_Widget *w, void *data);

    static void show_alert_callback(void *data);
    static void show_save_callback(void *data);
};

} // namespace pewter
