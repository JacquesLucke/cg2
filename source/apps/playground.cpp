#include "../app.hpp"

#include "../kdtree_viewer.hpp"

WindowController *createApplication(Window *window) {
    return new KDTreeViewer(window);
}
