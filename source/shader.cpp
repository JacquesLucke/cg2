#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader.hpp>
#include <iostream>

/* GLProgram
****************************************/

GLProgram::GLProgram(std::string vertexShader, std::string fragmentShader) {
    this->vertexShader = new GLVertexShader(vertexShader);
    this->fragmentShader = new GLFragmentShader(fragmentShader);
    this->programID = 0;
}

GLProgram::~GLProgram() {
    delete vertexShader;
    delete fragmentShader;
}

void GLProgram::compile() {
    vertexShader->compile();
    fragmentShader->compile();

    programID = glCreateProgram();
    glAttachShader(programID, vertexShader->shaderID);
    glAttachShader(programID, fragmentShader->shaderID);
    glLinkProgram(programID);
    glValidateProgram(programID);

    vertexShader->deleteCompiledResult();
    fragmentShader->deleteCompiledResult();
}



/* GLShader
****************************************/

GLShader::GLShader(std::string source) {
    this->source = source;
    this->shaderID = 0;
}

void GLShader::deleteCompiledResult() {
    glDeleteShader(shaderID);
    shaderID = 0;
}

static unsigned int compileShader(unsigned int type, const std::string& source);

void GLVertexShader::compile() {
    shaderID = compileShader(GL_VERTEX_SHADER, source);
}

void GLFragmentShader::compile() {
    shaderID = compileShader(GL_FRAGMENT_SHADER, source);
}



/* Utility
****************************************/

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