#include <GLFW/glfw3.h>
#include "kdtree.h"
#include "vector.h"
#include <iostream>

#define NDIM 3

template<int N>
using VectorKDTree = KDTree<Vector<N>, N, getVectorIndex<N>, Vector<N>::distance>;


int main(void)
{
    std::vector<Vector<NDIM>> points = generateRandomVectors<NDIM>(100'000, 42);
    // std::vector<Vector<NDIM>> points;
    // points.push_back(Vector<NDIM>());

    VectorKDTree<NDIM> tree(points.data(), points.size(), 10);
    tree.balance();
    return 0;

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}