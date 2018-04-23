#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "kdtree.h"
#include "vector.h"
#include <iostream>

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

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}