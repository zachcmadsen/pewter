#include <FL/Fl.H>

#include "app.hpp"

int main() {
    pewter::App *app = new pewter::App();
    app->show();

    Fl::lock();

    return Fl::run();
}
