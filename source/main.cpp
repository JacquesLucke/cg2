#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include "kdtree.h"
#include "vector.h"
#include <iostream>
#include <string>
#include "app.h"

#define NDIM 3

template<int N>
using VectorKDTree = KDTree<Vector<N>, N, getVectorIndex<N>, Vector<N>::distance>;

int main(void)
{
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    GLFWwindow *window = glfwCreateWindow(640, 480, "CG 2", NULL, NULL);
    if (window == nullptr)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;

    App *app = new TestApp(window);
    app->setup();

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        app->update();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    app->teardown();

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
