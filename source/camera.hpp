#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "window.hpp"
#include "ray.hpp"

class Camera {
public:
    glm::vec3 eye;
    glm::vec3 center;
    glm::vec3 upDirection;

    Camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
        : eye(eye), center(center), upDirection(up) {}

    virtual ~Camera(){}

    glm::mat4 getViewMatrix() const;
    virtual glm::mat4 getProjectionMatrix() const = 0;
    glm::mat4 getViewProjectionMatrix() const;

    void moveForward(float step);
    void moveBackward(float step);
    void moveRight(float step);
    void moveLeft(float step);
    void moveUp(float step);
    void moveDown(float step);
    void move(glm::vec3 offset);
    void zoom(float factor);

    void rotateHorizontal(float angle);
    void rotateVertical(float angle);

    void rotateHorizontalAroundCenter(float angle);
    void rotateVerticalAroundCenter(float angle);

    glm::vec3 direction() const;
    glm::vec3 right() const;
    glm::vec3 up() const;

    Ray getViewRay(glm::vec2 coords) const;
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

    glm::mat4 getProjectionMatrix() const;
};

class PerspectiveCamera : public Camera {
public:
    float fov;
    float aspect;
    float zNear;
    float zFar;

    PerspectiveCamera(glm::vec3 eye, glm::vec3 center, float fov, float aspect)
        : Camera(eye, center, glm::vec3(0, 1, 0)), fov(fov), aspect(aspect), zNear(0.1f), zFar(10000.0f) {}

    PerspectiveCamera(glm::vec3 eye, glm::vec3 center, glm::vec3 up,
                      float fov, float aspect, float zNear, float zFar)
        : Camera(eye, center, up), fov(fov), aspect(aspect), zNear(zNear), zFar(zFar) {}

    glm::mat4 getProjectionMatrix() const;
};

class CameraController {
public:
    Camera* camera;
    Window* window;

    CameraController(Camera* camera, Window* window)
        : camera(camera), window(window) {}

    void update(float elapsedMilliseconds);

private:
    bool isKeyDown(int key);
    bool isMouseDown(int key);
    glm::vec2 getMousePosition();

    glm::vec2 lastMousePosition;
};
