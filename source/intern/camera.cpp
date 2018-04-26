#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include "../camera.hpp"


/* Base Camera
**************************************/

glm::mat4 Camera::getViewProjectionMatrix() {
    return getProjectionMatrix() * getViewMatrix();
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(eye, center, up());
}

void Camera::move(glm::vec3 offset) {
    eye += offset;
    center += offset;
}

glm::vec3 Camera::direction() {
    glm::vec3 direction = center - eye;
    glm::normalize(direction);
    return direction;
}

glm::vec3 Camera::right() {
    glm::vec3 right = glm::cross(direction(), up());
    glm::normalize(right);
    return right;
}

glm::vec3 Camera::up() {
    glm::vec3 up = upDirection;
    glm::normalize(up);
    return up;
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



/* Orthographic Camera
****************************************/

glm::mat4 OrthographicCamera::getProjectionMatrix() {
    float wHalf = width / 2.0f;
    float hHalf = height / 2.0f;
    return glm::ortho(-wHalf, wHalf, -hHalf, hHalf, zNear, zFar);
}



/* Perspective Camera
******************************************/

glm::mat4 PerspectiveCamera::getProjectionMatrix() {
    return glm::perspective(fov, aspect, zNear, zFar);
}



/* Camera Controller
*******************************************/

// have to store these values globally to make the callback work
CameraController* flyingCamera;
GLFWscrollfun oldScrollCallback;

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    if (yOffset < 0) {
        flyingCamera->speed *= (float)pow(0.9f, abs(yOffset));
    } else {
        flyingCamera->speed *= (float)pow(1.1f, yOffset);
    }
}

void CameraController::enableFlyMode() {
    glfwSetInputMode(window->handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    oldScrollCallback = glfwSetScrollCallback(window->handle(), scrollCallback);
    lastMousePosition = getMousePosition();
    flyingCamera = this;
    mode = FLY;
}

void CameraController::disableFlyMode() {
    mode = FIXED;
    glfwSetInputMode(window->handle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetScrollCallback(window->handle(), oldScrollCallback);
    flyingCamera = NULL;
}

void CameraController::update() {
    if (mode != FLY) return;

    float step = 0.015f * speed;
    if (isKeyDown(GLFW_KEY_W)) camera->moveForward(step);
    if (isKeyDown(GLFW_KEY_S)) camera->moveBackward(step);
    if (isKeyDown(GLFW_KEY_A)) camera->moveLeft(step);
    if (isKeyDown(GLFW_KEY_D)) camera->moveRight(step);
    if (isKeyDown(GLFW_KEY_Q)) camera->moveDown(step * 4);
    if (isKeyDown(GLFW_KEY_E)) camera->moveUp(step * 4);

    float angleStep = 0.001f;
    glm::vec2 mousePos = getMousePosition();
    glm::vec2 mouseDiff = mousePos - lastMousePosition;
    camera->rotateHorizontal(-mouseDiff[0] * angleStep);
    camera->rotateVertical(-mouseDiff[1] * angleStep);

    lastMousePosition = mousePos;
}

bool CameraController::isKeyDown(int key) {
    return glfwGetKey(window->handle(), key) == GLFW_PRESS;
}

glm::vec2 CameraController::getMousePosition() {
    double x, y;
    glfwGetCursorPos(window->handle(), &x, &y);
    return glm::vec2(x, y);
}