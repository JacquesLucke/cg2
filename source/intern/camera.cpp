#include <iostream>
#include <cmath>

#include "../window.hpp"
#include "../camera.hpp"


/* Base Camera
**************************************/
float zoomZ = 0.0;
float tmp = 0.0;

void Camera::moveCam(void) {
	if (zoomZ != tmp )
		eye += glm::vec3( zoomZ, zoomZ, zoomZ);

	tmp = zoomZ;
}

glm::mat4 Camera::getViewProjectionMatrix() const {
    //return getProjectionMatrix() * getViewMatrix();
	glm::mat4 RotationX = glm::rotate(glm::mat4(1.0f), deltaX, up());// glm::vec3(0,1,0));
	glm::mat4 RotationY = glm::rotate(glm::mat4(1.0f), deltaY, right());// glm::vec3(1,0,0));
	return getProjectionMatrix() * getViewMatrix() * RotationX * RotationY;
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

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
	tmp = 0.0;

	if (yOffset < 0) {
		zoomZ = 0.1;
    } else {
    	zoomZ = -0.1;
    }
}

void CameraController::initCB() {
    glfwSetScrollCallback(window->handle(), scrollCallback);
}

void CameraController::update(int triggerKey, float elapsedMilliseconds) {
	static int initScrollCB = 0;
	if (initScrollCB == 0)
	{
		initCB();
		initScrollCB = 1;
	}

    if (isKeyDown(GLFW_KEY_ESCAPE)) {
    	// end program
        exit (0);
    }

    CameraController::camera->moveCam();

    glm::vec2 mousePos = getMousePosition();
    glm::vec2 angleDiff = (mousePos - lastMousePosition) * 0.001f;
    lastMousePosition = mousePos;

    if (isKeyDown(GLFW_KEY_LEFT_CONTROL) && CameraController::getMouseButton()==1)
    {
    	CameraController::camera->deltaX += 2*angleDiff.x;
    	CameraController::camera->deltaY += 2*angleDiff.y;
    }

	#if 0
		float step = 0.001f * speed * elapsedMilliseconds;
		if (isKeyDown(GLFW_KEY_W)) camera->moveForward(step);
		if (isKeyDown(GLFW_KEY_S)) camera->moveBackward(step);
		if (isKeyDown(GLFW_KEY_A)) camera->moveLeft(step);
		if (isKeyDown(GLFW_KEY_D)) camera->moveRight(step);
		if (isKeyDown(GLFW_KEY_Q)) camera->moveDown(step * 3);
		if (isKeyDown(GLFW_KEY_E)) camera->moveUp(step * 3);


		if (isKeyDown(GLFW_KEY_LEFT))
		{
			printf(" Left mouse down\n");
		}

		if (lastMousePosition.x != 0 || lastMousePosition.y != 0)
			printf("lastMousePosition x: %.2f  y: %.2f\n",angleDiff.x, angleDiff.y );


		int button = CameraController::getMouseButton();
		if (button != 0)
		{
			/* left Mouse button == 1
			 * right Mouse button == 2
			 * middle mouse button == 4
			 */
			printf("Mouse button: %d\n",button);
		}


		if (isKeyDown(GLFW_KEY_RIGHT_CONTROL))
		{
			CameraController::camera->deltaY += 0.01;
			printf(" Right Ctrl Key down\n");
		}

		if ( isKeyDown(GLFW_KEY_LEFT) && isKeyDown(GLFW_KEY_LEFT_CONTROL) )
		{
			printf(" Left Mouse & Left Ctrl Key down\n");
		}

		if ( isKeyDown(GLFW_KEY_LEFT) && isKeyDown(GLFW_KEY_RIGHT_CONTROL) )
		{
			printf(" Left Mouse & Right Ctrl Key down\n");
		}



		float angleStep = 0.001f * elapsedMilliseconds;
		if (isKeyDown(GLFW_KEY_LEFT)) angleDiff.x -= angleStep;
		if (isKeyDown(GLFW_KEY_RIGHT)) angleDiff.x += angleStep;
		if (isKeyDown(GLFW_KEY_UP)) angleDiff.y -= angleStep;
		if (isKeyDown(GLFW_KEY_DOWN)) angleDiff.y += angleStep;

		camera->rotateHorizontal(-angleDiff[0]);
		camera->rotateVertical(-angleDiff[1]);
	#endif
}

bool CameraController::isKeyDown(int key) {
    return glfwGetKey(window->handle(), key) == GLFW_PRESS;
}

glm::vec2 CameraController::getMousePosition() {
    double x, y;
    glfwGetCursorPos(window->handle(), &x, &y);
    return glm::vec2(x, y);
}

int CameraController::getMouseButton( void )
{
	/* left Mouse button == 1
	 * right Mouse button == 2
	 * middle mouse button == 4 */

	/* check only for mouse left mouse button (index 0) */
	return glfwGetMouseButton(window->handle(), 0);

	/* optional, poll for all mouse buttons in loop and code results */
	#if 0
		int ret = 0;
		for (int i = 0; i < 3; i++)
		{
			if (glfwGetMouseButton(window->handle(), i) != 0)
				ret = 1 << i;
		}
		return ret;
	#endif
}



