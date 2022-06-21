#include "camera.hpp"

#include <GLFW/glfw3.h>

Camera::Camera(glm::vec3 const &position)
		: position(position) {}

void Camera::updateMouse(glm::vec2 const &relative)
{
	rotation.x += relative.y * sensitivity.x;
	rotation.y += relative.x * sensitivity.y;
	rotation.x = glm::clamp(rotation.x, -89.0f, 89.0f);
	rotation.y = glm::mod(rotation.y, 360.0f);

	yawOrientation = glm::angleAxis(glm::radians(-rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	pitchOrientation = glm::angleAxis(glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
}

void Camera::updateKey(int key)
{
	/* FPS Camera */
	// glm::quat orientation = yawOrientation * glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	/* Free Camera */
	glm::quat orientation = yawOrientation * pitchOrientation;
	glm::quat quatFront = orientation * glm::quat(0.0f, 0.0f, 0.0f, -1.0f) * glm::conjugate(orientation);
	glm::vec3 front(quatFront.x, quatFront.y, quatFront.z);
	glm::vec3 right = glm::normalize(glm::cross(front, UP));

	if (key == GLFW_KEY_W)
	{
		direction = front;
	}
	if (key == GLFW_KEY_S)
	{
		direction = -front;
	}

	if (key == GLFW_KEY_A)
	{
		direction = -right;
	}
	if (key == GLFW_KEY_D)
	{
		direction = right;
	}
}

[[nodiscard]] glm::mat4 Camera::viewMatrix() const
{
	glm::mat4 viewRotation = glm::mat4_cast(glm::conjugate(yawOrientation * pitchOrientation));
	glm::mat4 viewTranslation = glm::translate(glm::mat4(1.0f), -position);
	return viewRotation * viewTranslation;
}