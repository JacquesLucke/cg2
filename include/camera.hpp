#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 eye;
    glm::vec3 center;
    glm::vec3 up;

    Camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
        : eye(eye), center(center), up(up) {}

    glm::mat4 getViewProjectionMatrix();

    glm::mat4 getViewMatrix();
    virtual glm::mat4 getProjectionMatrix() = 0;
};

class OrthographicCamera : public Camera {
public:
    float width;
    float height;
    float zNear;
    float zFar;

    OrthographicCamera(glm::vec3 eye, glm::vec3 center, glm::vec3 up,
                       float width, float height, float zNear, float zFar)
        : Camera(eye, center, up), width(width), height(height), zNear(zNear), zFar(zFar) {}

    glm::mat4 getProjectionMatrix();
};

class PerspectiveCamera : public Camera {
public:
    float fov;
    float aspect;
    float zNear;
    float zFar;

    PerspectiveCamera(glm::vec3 eye, glm::vec3 center, glm::vec3 up,
                      float fov, float aspect, float zNear, float zFar)
        : Camera(eye, center, up), fov(fov), aspect(aspect), zNear(zNear), zFar(zFar) {}

    glm::mat4 getProjectionMatrix();
};