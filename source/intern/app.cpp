#include <cassert>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../app.hpp"
#include "../camera.hpp"


float color[4] = {1, 1, 1, 1};


namespace cgX
{
    App::~App() {}

    bool App::setup(const std::string& name, const Config& config) {
        if(!_window.setup(name, config))
          return false;

        ImGui::CreateContext();
        ImGui_ImplGlfwGL3_Init(_window.handle(), true);
        ImGui::StyleColorsDark();

        return onSetup();
    }

    void App::teardown() {
        onTeardown();

        ImGui_ImplGlfwGL3_Shutdown();
        ImGui::DestroyContext();

        _window.terminate();
    }

    void App::update() {
        onUpdate();
    }

    void App::render() {
        _window.beginFrame();

        onRender();

        ImGui_ImplGlfwGL3_NewFrame();
        onRenderUI();
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

        _window.endFrame();
    }

    bool App::running() const {
        return !_window.shouldClose();
    }



    /**
       Testapp
    **/

    bool TestApp::onSetup() {
        program = GLProgram::FromFile("C:\\Users\\jacques\\Desktop\\cg2-git\\testshader.shader");
        program->compile();

        offData = readOffFile("C:\\Users\\jacques\\Downloads\\cg2_ex1\\off_files\\dragon.off");
        //printVectors(offData->positions);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, offData->positions.size() * sizeof(Vector<3>), offData->positions.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 3, 0);
        glEnableVertexAttribArray(0);

        return true;
    }

    void TestApp::onRender() {
        GLFWwindow *w = window().handle();
        ImGuiIO& io = ImGui::GetIO();
        bool wantskeyboard = io.WantCaptureKeyboard;
        if (glfwGetKey(w, GLFW_KEY_F) == GLFW_PRESS && !wantskeyboard) {
            std::cout << "Down" << std::endl;
        }

        if (program == nullptr) glUseProgram(0);
        else {
            program->use();
            program->setUniform4f("u_Color", color);

            OrthographicCamera camera1(glm::vec3(3, -1, -2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 2, 2, 0.1, 20);
            PerspectiveCamera camera2(glm::vec3(150, 200, 150), glm::vec3(0, 50, 0), glm::vec3(0, 1, 0), 1.5, 1, 0.1, 2000);
            program->setUniformMat4f("u_MVP", camera2.getViewProjectionMatrix());
        }

        glPointSize(1);
        glDrawArrays(GL_POINTS, 0, offData->positions.size());
    }

    void TestApp::onRenderUI() {
        static char path[200];
        ImGui::InputText("Path", path, sizeof(path));
        ImGui::ColorEdit3("Color", color);

        if (ImGui::Button("Load Shader")) {
            delete program;
            program = GLProgram::FromFile(path);
            program->compile();
        }
    }
}
