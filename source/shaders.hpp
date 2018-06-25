#pragma once

#include "shader.hpp"
#include "glm/glm.hpp"

class Shader {
public:
    void bind() const;
    void setViewProj(glm::mat4 matViewProjection);
    void setModelMatrix(glm::mat4 matModel);
    void resetModelMatrix();

    int getAttributeLocation_Position() const;
    int getAttributeLocation_Normal() const;
    int getAttributeLocation_Color() const;

protected:
    GLProgram *shader;
};

class FlatShader : public Shader {
public:
    FlatShader();
    void setColor(glm::vec4 color);
    void setColor(float r, float g, float b, float a = 1.0f);
};

class SolidShader : public Shader {
public:
    SolidShader();
    void setBrightness(float brightness);
};

class ShadelessColorShader : public Shader {
public:
    ShadelessColorShader();
};

class BlinnPhongShader : public Shader {
public:
    BlinnPhongShader();

    void setAmbientColor(float r, float g, float b);
    void setSpecularColor(float r, float g, float b);
    void setDiffuseColor(float r, float g, float b);
    void setCameraPosition(const glm::vec3 &pos);
    void setLightPosition(const glm::vec3 &pos);

    void setMagnitude(float value);
};