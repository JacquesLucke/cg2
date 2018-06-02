#pragma once

#include <string>
#include <glm/glm.hpp>

class GLShader {
public:
    unsigned int shaderID;
    std::string source;

    GLShader(std::string source);
    virtual ~GLShader();

    virtual void compile() = 0;
    virtual void deleteCompiledResult();
};

class GLVertexShader : public GLShader {
public:
    GLVertexShader(std::string source) : GLShader(source) {};

    void compile() final override;
};

class GLFragmentShader : public GLShader {
public:
    GLFragmentShader(std::string source) : GLShader(source) {};

    void compile() final override;
};

class GLProgram {
public:
    unsigned int programID;
    GLVertexShader *vertexShader;
    GLFragmentShader *fragmentShader;

    GLProgram(std::string vertexShader, std::string fragmentShader);
    ~GLProgram();

    static GLProgram *FromFile(std::string path);

    void compile();
    void bind();

    void setUniform1f(const std::string& name, float value);
    void setUniform4f(const std::string& name, float v1, float v2, float v3, float v4);
    void setUniform4f(const std::string& name, float* value);
    void setUniformMat4f(const std::string& name, const glm::mat4& matrix);

    int getAttributeLocation(const std::string& name) const;

private:
    int getUniformLocation(const std::string& name) const;
};
