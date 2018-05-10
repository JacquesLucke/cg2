#include "../shaders.hpp"
#include "../resources.hpp"

GLProgram *loadShader(const std::string name) {
    GLProgram *shader = loadRelShaderResource(name);
    assert(shader != nullptr);
    shader->compile();
    return shader;
}


void Shader::bind() {
    shader->bind();
}

void Shader::setTransforms(glm::mat4 matViewProjection, glm::mat4 matModel) {
    shader->setUniformMat4f("u_ViewProjection", matViewProjection);
    shader->setUniformMat4f("u_Model", matModel);
}

int Shader::getAttributeLocation_Position() const {
    return shader->getAttributeLocation("position");
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
