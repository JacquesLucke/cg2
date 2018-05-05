#pragma once

#include "ogl.hpp"

#include <string>
#include "mesh.hpp"

enum class RENDER_MODE
{
    SOLID,
    WIREFRAME,
    POINTS
};

class Window {
public:
    static Window *TryCreateNew(const std::string& title, int width, int height){
        GLFWwindow* handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (handle == nullptr) {
            return nullptr;
        } else {
            return new Window(handle);
        }
    }

    Window(GLFWwindow* handle);
    ~Window();


    template<typename TVertex>
    void render(Mesh<TVertex> *mesh) {
	mesh->bindBuffers();
	onRender(mesh->getIndexCount());
	mesh->unbindBuffers();
    }

    void setRenderMode(RENDER_MODE mode);
    void setPointSize(int pointSize);

    void activateContext();
    bool shouldClose() const;
    void beginFrame();
    void endFrame();

    float aspect();
    int width();
    int height();

    GLFWwindow* handle() { return _handle; }

private:
    void onRender(unsigned int indexCount);
    
    GLFWwindow* _handle;
    unsigned int drawFlag;
};
