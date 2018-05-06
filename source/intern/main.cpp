#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <iostream>
#include <string>

#include "../ogl.hpp"

#include "../kdtree_viewer.hpp"
#include "../vector.hpp"
#include "../kdtree.hpp"
#include "../window.hpp"

#include "../resources.hpp"

#include "../app.hpp"

// #define NDIM 3

// template<int N>
// using VectorKDTree = KDTree<Vector<N>, N, getVectorIndex<N>, Vector<N>::distance>;


int main(int argc, char *argv[]) {
    Resources::init(argc, argv);
    
    if(!glfwInit()) {
        return EXIT_FAILURE;
    }

    Window* window = Window::TryCreateNew("My Test", 800, 600);
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
