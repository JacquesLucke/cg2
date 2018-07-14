#include "../app.hpp"

#include "../mesh_smoothing_viewer.hpp"

WindowController *createApplication(Window *window) {
    return new MeshSmoothingViewer(window);
}
