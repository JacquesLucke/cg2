#pragma once

#include "window_controller.hpp"

class ParametricSurfaceViewer : public WindowController {
public:
    ParametricSurfaceViewer(Window* window)
        : WindowController(window) {}

protected:
    bool onSetup() final override;
};