#include <camera.hpp>


/* Base Camera
**************************************/

glm::mat4 Camera::getViewProjectionMatrix() {
    return getProjectionMatrix() * getViewMatrix();
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(eye, center, up);
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