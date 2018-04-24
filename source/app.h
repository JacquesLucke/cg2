#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class App {
public:
    GLFWwindow *window;

    App(GLFWwindow *window) {
        this->window = window;
    }

    virtual void setup() {}
    virtual void teardown() {}
    virtual void update() {}
};

class TestApp : public App {
public:
    using App::App;

    void setup() {
        ImGui::CreateContext();
        ImGui_ImplGlfwGL3_Init(window, true);
        ImGui::StyleColorsDark();
    }

    void teardown() {
        ImGui_ImplGlfwGL3_Shutdown();
        ImGui::DestroyContext();
    }

    void update() {
        ImGui_ImplGlfwGL3_NewFrame();
        ImGui::Text("Test App");
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }
};
