#include "../shaders.hpp"
#include "../resources.hpp"

GLProgram *loadShader(const std::string name) {
    GLProgram *shader = loadRelShaderResource(name);
    assert(shader != nullptr);
    shader->compile();
    return shader;
}


void Shader::bind() const {
    shader->bind();
}

void Shader::setViewProj(glm::mat4 matViewProjection) {
    shader->setUniformMat4f("u_ViewProjection", matViewProjection);
}

void Shader::resetModelMatrix() {
    setModelMatrix(glm::mat4(1.0f));
}

void Shader::setModelMatrix(glm::mat4 matModel) {
    shader->setUniformMat4f("u_Model", matModel);
}

int Shader::getAttributeLocation_Position() const {
    return shader->getAttributeLocation("position");
}

int Shader::getAttributeLocation_Normal() const {
    return shader->getAttributeLocation("normal");
}


FlatShader::FlatShader() {
    shader = loadShader("flat.shader");
}

void FlatShader::setColor(glm::vec4 color) {
    shader->setUniform4f("u_Color", (float*)&color);
}

void FlatShader::setColor(float r, float g, float b, float a) {
    setColor(glm::vec4(r, g, b, a));
}


SolidShader::SolidShader() {
    shader = loadShader("solid.shader");
}

void SolidShader::setBrightness(float brightness) {
    shader->setUniform1f("u_Brightness", brightness);
}