// TODO: Turn the include header diagnostics back on.
#include <FL/Fl.H>
#include <fmt/core.h>

#include "app.h"

int main() {
    App *app = new App();
    app->show();

    return Fl::run();
}
