#pragma once

#include "shader.hpp"
#include "glm/glm.hpp"

class Shader {
public:
    void bind();
    void setTransforms(glm::mat4 matViewProjection, glm::mat4 matModel = glm::mat4(1));

    int getAttributeLocation_Position() const;

protected:
    GLProgram *shader;
};

class FlatShader : public Shader {
public:
    FlatShader();
    void setColor(glm::vec4 color);
    void setColor(float r, float g, float b, float a = 1.0f);
};