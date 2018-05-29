#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

float vertices[6] = {
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

    virtual ~App() {};

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

    unsigned int VBO, VAO;

    void setup() {
        ImGui::CreateContext();
        ImGui_ImplGlfwGL3_Init(window, true);
        ImGui::StyleColorsDark();
/*new*/ glGenVertexArrays(1, &VAO);
    	glGenBuffers(1, &VBO);
    	// bind the Vertex Array Object first,
    	// then bind and set vertex buffer(s),
    	// and then configure vertex attributes(s).
/*new*/ glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

 /*new*/ // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
 /*new*/ glBindBuffer(GL_ARRAY_BUFFER, 0);
 /*new*/
 /*new*/ // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
 /*new*/ // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
 /*new*/ glBindVertexArray(0);

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
            "   color = vec4(0.5, 0.0, 0.5, 1.0);\n"
            "}\n\0";

        unsigned int shader = createShader(vertexShader, fragmentShader);
        glUseProgram(shader);
    }

    void teardown() {
        // optional: de-allocate all resources once they've outlived their purpose:
        // ------------------------------------------------------------------------
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        ImGui_ImplGlfwGL3_Shutdown();
        ImGui::DestroyContext();
    }

    void update() {
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawArrays(GL_TRIANGLES, 0, 3);
        ImGui_ImplGlfwGL3_NewFrame();
        ImGui::Text("Test App");
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        // glBindVertexArray(0); // no need to unbind it every time

    }
};
