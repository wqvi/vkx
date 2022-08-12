#include <glm/ext/quaternion_geometric.hpp>
#include <vkx/camera.hpp>

#include <SDL2/SDL_keycode.h>

vkx::Camera::Camera(const glm::vec3& position)
    : position(position) {}

void vkx::Camera::updateMouse(const glm::vec2& relative) {
	rotation.x += relative.y * sensitivity.x;
	rotation.y += relative.x * sensitivity.y;
	rotation.x = glm::clamp(rotation.x, -89.0f, 89.0f);
	rotation.y = glm::mod(rotation.y, 360.0f);

	yawOrientation = glm::angleAxis(glm::radians(-rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	pitchOrientation = glm::angleAxis(glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
}

void vkx::Camera::updateKey(SDL_Keycode key) {
	// FPS Camera
	// glm::quat orientation = yawOrientation * glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	// Free Camera
	glm::quat orientation = yawOrientation * pitchOrientation;
	glm::quat quaternionFront = orientation * glm::quat(0.0f, 0.0f, 0.0f, -1.0f) * glm::conjugate(orientation);
	glm::vec3 front(quaternionFront.x, quaternionFront.y, quaternionFront.z);
	glm::vec3 right = glm::normalize(glm::cross(front, vkx::UP));

	// TODO fix this monstrosity
	if (key == SDLK_w) {
		direction = front;
	}
	if (key == SDLK_s) {
		direction = -front;
	}
	if (key == SDLK_a) {
		direction = -right;
	}
	if (key == SDLK_d) {
		direction = right;
	}

	direction = glm::normalize(direction);
}

[[nodiscard]] glm::mat4 vkx::Camera::viewMatrix() const {
	glm::mat4 viewRotation = glm::mat4_cast(glm::conjugate(yawOrientation * pitchOrientation));
	glm::mat4 viewTranslation = glm::translate(glm::mat4(1.0f), -position);
	return viewRotation * viewTranslation;
}