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

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}
void processInput(GLFWwindow *window);

int main(void)
{
    // Setup window
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(640, 480, "CG 2", NULL, NULL);
    if (window == nullptr)
    {
    	std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;

    App *app = new TestApp(window);
    app->setup();

    while (!glfwWindowShouldClose(window))
    {
    	// input
    	// -----
    	processInput(window);
        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT);
        app->update();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    app->teardown();

    glfwDestroyWindow(window);
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();

    exit(EXIT_SUCCESS);
}

// process all input: query GLFW whether relevant keys are pressed/released
// this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

