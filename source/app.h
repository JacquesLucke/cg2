#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

float positions[6] = {
    -0.5f, -0.5f,
     0.0f,  0.5f,
     0.5f, -0.5f
};

class App {
public:
    GLFWwindow* window;

    App(GLFWwindow* window) {
        this->window = window;
    }

    virtual void setup() {}
    virtual void teardown() {}
    virtual void update() {}
};

static unsigned int compileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Shader Compilation failed:" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    assert(vs > 0);
    assert(fs > 0);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

class TestApp : public App {
public:
    using App::App;

    unsigned int buffer;

    void setup() {
        ImGui::CreateContext();
        ImGui_ImplGlfwGL3_Init(window, true);
        ImGui::StyleColorsDark();

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
        unsigned int shader = createShader(vertexShader, fragmentShader);
        glUseProgram(shader);
    }

    void teardown() {
        ImGui_ImplGlfwGL3_Shutdown();
        ImGui::DestroyContext();
    }

    void update() {
        glDrawArrays(GL_TRIANGLES, 0, 3);
        ImGui_ImplGlfwGL3_NewFrame();
        ImGui::Text("Test App");
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }
};
