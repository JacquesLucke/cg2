#pragma once

#include "window.hpp"
#include "window_controller.hpp"

// Hook function for all apps
WindowController *createApplication(Window *window);
