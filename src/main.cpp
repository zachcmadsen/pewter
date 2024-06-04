#include <FL/Fl.H>

#include "app.hpp"

int main() {
    pewter::App *app = new pewter::App();
    app->show();

    return Fl::run();
}
