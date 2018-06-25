#include <iostream>
#include <cmath>

#include "../window.hpp"
#include "../camera.hpp"


/* Base Camera
**************************************/

glm::mat4 Camera::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(eye, center, up());
}

void Camera::move(glm::vec3 offset) {
    eye += offset;
    center += offset;
}

glm::vec3 Camera::direction() const {
    glm::vec3 direction = center - eye;
    return glm::normalize(direction);
}

glm::vec3 Camera::right() const {
    glm::vec3 right = glm::cross(direction(), up());
    return glm::normalize(right);
}

glm::vec3 Camera::up() const {
    glm::vec3 up = upDirection;
    return glm::normalize(up);
}

void Camera::moveForward(float step) { move(direction() * step); }
void Camera::moveBackward(float step) { moveForward(-step); }
void Camera::moveRight(float step) { move(right() * step); }
void Camera::moveLeft(float step) { moveRight(-step); }
void Camera::moveUp(float step) { move(up() * step); }
void Camera::moveDown(float step) { moveUp(-step); }

static glm::vec3 rotateVector(glm::vec3 vector, glm::vec3 axis, float angle) {
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);
    return glm::vec3(rotation * glm::vec4(vector, 1.0f));
}

void Camera::rotateHorizontal(float angle) {
    glm::vec3 direction = center - eye;
    center = eye + rotateVector(direction, up(), angle);
}

void Camera::rotateVertical(float angle) {
    glm::vec3 direction = center - eye;
    center = eye + rotateVector(direction, right(), angle);
}

void Camera::rotateHorizontalAroundCenter(float angle) {
    glm::vec3 direction = center - eye;
    direction = rotateVector(direction, up(), angle);
    eye = center - direction;
}

void Camera::rotateVerticalAroundCenter(float angle) {
    glm::vec3 direction = center - eye;
    direction = rotateVector(direction, right(), angle);
    eye = center - direction;
}

// x and y must be in [-1, 1]
Ray Camera::getViewRay(glm::vec2 coords) const {
    glm::mat4 matrixViewProj = getViewProjectionMatrix();
    glm::mat4 invViewProj = glm::inverse(matrixViewProj);
    glm::vec4 screenPosition = glm::vec4(coords.x, -coords.y, 1.0f, 1.0f);
    glm::vec4 worldPosition = invViewProj * screenPosition;
    glm::vec3 direction = glm::normalize(glm::vec3(worldPosition));
    return Ray(eye, direction);
}



/* Orthographic Camera
****************************************/

glm::mat4 OrthographicCamera::getProjectionMatrix() const {
    float wHalf = width / 2.0f;
    float hHalf = height / 2.0f;
    return glm::ortho(-wHalf, wHalf, -hHalf, hHalf, zNear, zFar);
}



/* Perspective Camera
******************************************/

glm::mat4 PerspectiveCamera::getProjectionMatrix() const {
    return glm::perspective(fov, aspect, zNear, zFar);
}

/* Camera Controller
*******************************************/

void CameraController::update(float elapsedMilliseconds) {
    glm::vec2 mousePos = getMousePosition();
    glm::vec2 mouseDiff = mousePos - lastMousePosition;
    lastMousePosition = mousePos;

    if (isMouseDown(GLFW_MOUSE_BUTTON_LEFT)) {
        float rotateSpeed = 0.004f;
        camera->rotateHorizontalAroundCenter(-mouseDiff.x * rotateSpeed);
        camera->rotateVerticalAroundCenter(-mouseDiff.y * rotateSpeed);
    }
    if (isMouseDown(GLFW_MOUSE_BUTTON_RIGHT)) {
        float moveSpeed = 0.01f;
        camera->moveLeft(mouseDiff.x * moveSpeed);
        camera->moveUp(mouseDiff.y * moveSpeed);
    }
}

bool CameraController::isKeyDown(int key) {
    return glfwGetKey(window->handle(), key) == GLFW_PRESS;
}

bool CameraController::isMouseDown(int key) {
    return glfwGetMouseButton(window->handle(), key) == GLFW_PRESS;
}

glm::vec2 CameraController::getMousePosition() {
    double x, y;
    glfwGetCursorPos(window->handle(), &x, &y);
    return glm::vec2(x, y);
}

