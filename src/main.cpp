#include <FL/Fl.H>

#include "app.hpp"

int main()
{
    Fl::lock();

    pewter::App app;
    app.show();

    return Fl::run();
}
