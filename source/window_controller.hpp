#pragma once

#include <string>
#include "window.hpp"

#include "glm/vec2.hpp"

class WindowController {
public:
    WindowController(Window* window)
        : _window(window) {}

    virtual ~WindowController() {};

    bool setup();
    void update();
    void render();
    void teardown();
    bool isRunning() const;

    glm::vec2 getMousePos();

    virtual bool onSetup() { return true; };
    virtual void onTeardown() {}
    virtual void onUpdate() {}
    virtual void onRender() {}
    virtual void onRenderUI() {}

    Window *window() { return _window; }

protected:
    Window* _window;
};
