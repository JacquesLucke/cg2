#include "../app.hpp"

#include "../implicit_surface_viewer.hpp"

WindowController *createApplication(Window *window) {
    return new ImplicitSurfaceViewer(window);
}
