/* install glfw:
    https://github.com/glfw/glfw/issues/808

    deb http://ppa.launchpad.net/keithw/glfw3/ubuntu trusty main

    Then execute these commands

    sudo apt-get update
    sudo apt-get install libglfw3
    sudo apt-get install libglfw3-dev

    The library should now be available here:

    /usr/lib/x86_64-linux-gnu/libglfw.so
    /usr/lib/x86_64-linux-gnu/libglfw.so.3
    /usr/lib/x86_64-linux-gnu/libglfw.so.3.1

    And the headers here:

    /usr/include/GLFW/glfw3.h
 */

/* link against libglfw3 libGL
    -lglfw3 -lGL
*/

/* Example show only empty window
   and does not use 'glad' */


#include <GLFW/glfw3.h>

int main(void)
{
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
