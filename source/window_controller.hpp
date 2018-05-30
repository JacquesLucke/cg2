#pragma once

#include <string>
#include <chrono>
#include <glm/glm.hpp>

#include "window.hpp"

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
    bool isKeyDown(int key);

    virtual bool onSetup() { return true; };
    virtual void onTeardown() {}
    virtual void onUpdate() {}
    virtual void onRender() {}
    virtual void onRenderUI() {}

    Window *window() { return _window; }

protected:
    float getElapsedMilliseconds();

    Window* _window;

private:
    void updateElapsedTime();

    double lastUpdateTime;
    double elapsedTime;
};
