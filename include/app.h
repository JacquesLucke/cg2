#pragma once

#include <iostream>
#include <window.hpp>
#include <shader.hpp>

namespace cgX
{
    class App {
    public:
        virtual ~App() = 0;

        bool setup(const std::string& windowName, const Config& config);
        void teardown();

        void update();
        void render();

        bool running() const;

        const Window& window() const { return _window; }

    protected:
        virtual bool onSetup() { return true; }
        virtual void onTeardown() { }

        virtual void onUpdate() { }
        virtual void onRender() { }
        virtual void onRenderUI() { }

    private:
        Window _window;
    };


    class TestApp : public App {
    protected:
        bool onSetup() final override;

        void onRender() final override;
        void onRenderUI() final override;

    private:
        unsigned int buffer;
        GLProgram *program;
    };
}