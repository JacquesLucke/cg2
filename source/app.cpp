#include <app.h>

#include <cassert>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>


float positions[6] = {
    -0.5f, -0.5f,
     0.0f,  0.5f,
     0.5f, -0.5f
};



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
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 2, 0);
        glEnableVertexAttribArray(0);

        std::string vertexShader =
            "#version 330 core\n"
            "\n"
            "layout(location = 0) in vec4 position;\n"
            "\n"
            "void main() {\n"
            "    gl_Position = position;\n"
            "}\n";

        std::string fragmentShader =
            "#version 330 core\n"
            "\n"
            "out vec4 color;\n"
            "\n"
            "void main() {\n"
            "   color = vec4(1.0, 1.0, 0.0, 1.0);\n"
            "}\n";

        program = new GLProgram(vertexShader, fragmentShader);
        program->compile();

        return true;
    }

    void TestApp::onRender() {
        glUseProgram(program->programID);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void TestApp::onRenderUI() {
        ImGui::Text("Test App");
    }
}
