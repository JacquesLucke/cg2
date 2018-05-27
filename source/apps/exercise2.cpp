#include "../app.hpp"

#include "../parametric_surface_viewer.hpp"

WindowController *createApplication(Window *window) {
    return new ParametricSurfaceViewer(window);
}
