#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <iostream>
#include <string>

#include "../ogl.hpp"
#include "../window.hpp"
#include "../app.hpp"
#include "../resources.hpp"



int main(int argc, char *argv[]) {
    Resources::init(argc, argv);

    if(!glfwInit()) {
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    Window* window = Window::TryCreateNew("My Test", 1200, 600);
    if (window == nullptr) {
        return EXIT_FAILURE;
    }

    window->activateContext();
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;

    // Call the application hook
    WindowController *controller = createApplication(window);
    if (nullptr == controller || !controller->setup()) {
        std::cout << "Setup failed" << std::endl;
        return EXIT_FAILURE;
    }

    while (controller->isRunning()) {
        controller->update();
        controller->render();
    }

    controller->teardown();
    delete controller;
    delete window;
    glfwTerminate();
    return EXIT_SUCCESS;
}
