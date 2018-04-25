#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace cgX
{
    struct Config {
        Config()
            : xRes(0), yRes(0) {}

        Config(int xRes, int yRes)
            : xRes(xRes), yRes(yRes) {}

        int xRes, yRes;
    };

    class Window {
    public:
        Window();
        ~Window();

        bool setup(const std::string& name, const Config& config);
        void terminate();

        bool good() const;

        bool shouldClose() const;

        void beginFrame();
        void endFrame();

        GLFWwindow* handle() { return window; }

    private:
        GLFWwindow* window;
        std::string name;
        Config config;
    };
}
