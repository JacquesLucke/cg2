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

int Shader::getAttributeLocation_Color() const {
    return shader->getAttributeLocation("color");
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


NormalShader::NormalShader() {
    shader = loadShader("normal.shader");
}

void NormalShader::setBrightness(float brightness) {
    shader->setUniform1f("u_Brightness", brightness);
}


ShadelessColorShader::ShadelessColorShader() {
    shader = loadShader("shadeless_color.shader");
}

BlinnPhongShader::BlinnPhongShader() {
    shader = loadShader("blinnphong.shader");
}

void BlinnPhongShader::setAmbientColor(float r, float g, float b) {
    shader->setUniform3f("ambientColor", r, g, b);
}

void BlinnPhongShader::setSpecularColor(float r, float g, float b) {
    shader->setUniform3f("specularColor", r, g, b);
}

void BlinnPhongShader::setDiffuseColor(float r, float g, float b) {
    shader->setUniform3f("diffuseColor", r, g, b);
}

void BlinnPhongShader::setCameraPosition(const glm::vec3 &pos) {
    shader->setUniform3f("cameraPosition", pos);
}

void BlinnPhongShader::setLightPosition(const glm::vec3 &pos) {
    shader->setUniform3f("lightPosition", pos);
}

void BlinnPhongShader::setMagnitude(float value) {
    shader->setUniform1f("magnitude", value);
}
