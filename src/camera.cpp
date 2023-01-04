#include <glm/ext/quaternion_geometric.hpp>
#include <glm/trigonometric.hpp>
#include <vkx/camera.hpp>

#include <SDL2/SDL_keycode.h>

vkx::Camera::Camera(const glm::vec3& position)
    : position(position) {}

void vkx::Camera::updateMouse(const glm::vec2& relative) {
	pitch += relative.y * sensitivity.x;
	yaw += relative.x * sensitivity.y;
	pitch = glm::clamp(rotation.x, -89.0f, 89.0f);
	yaw = glm::mod(rotation.y, 360.0f);

	const auto pitchRadians = glm::radians(pitch);
	const auto yawRadians = glm::radians(-yaw);

	pitchOrientation = glm::angleAxis(pitchRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	yawOrientation = glm::angleAxis(yawRadians, glm::vec3(0.0f, 1.0f, 0.0f));

	const auto orientation = yawOrientation * pitchOrientation;
	const auto quaternionFront = orientation * glm::quat(0.0f, 0.0f, 0.0f, -1.0f) * glm::conjugate(orientation);
	front = glm::normalize(glm::vec3{quaternionFront.x, quaternionFront.y, -quaternionFront.z});
}

void vkx::Camera::updateKey(SDL_Keycode key) {
	// FPS Camera
	// const auto orientation = yawOrientation * glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	// Free Camera
	const auto orientation = yawOrientation * pitchOrientation;
	const auto quaternionFront = orientation * glm::quat(0.0f, 0.0f, 0.0f, -1.0f) * glm::conjugate(orientation);
	front = glm::normalize(glm::vec3{quaternionFront.x, quaternionFront.y, quaternionFront.z});
	const glm::vec3 right = glm::normalize(glm::cross(front, vkx::UP));

	// TODO fix this monstrosity
	if (key == SDLK_w) {
		direction = -front;
		return;
	}
	if (key == SDLK_s) {
		direction = front;
		return;
	}
	if (key == SDLK_a) {
		direction = -right;
		return;
	}
	if (key == SDLK_d) {
		direction = right;
		return;
	}

	front.z = -front.z;
}

[[nodiscard]] glm::mat4 vkx::Camera::viewMatrix() const {
	const auto viewRotation = glm::mat4_cast(glm::conjugate(yawOrientation * pitchOrientation));
	const auto viewTranslation = glm::translate(glm::mat4(1.0f), -position);
	return viewRotation * viewTranslation;
}

vkx::Camera2D::Camera2D(const glm::vec2& position,
			const glm::vec2& rotation,
			const glm::vec2& mouseSensitivity)
    : position(position), rotation(rotation), mouseSensitivity(mouseSensitivity) {}

glm::mat4 vkx::Camera2D::viewMatrix() const noexcept {
	const auto viewRotation = glm::mat3_cast(glm::conjugate(yawOrientation * pitchOrientation));
	const auto viewTranslation = glm::translate(glm::mat3(1), -position);
	return viewRotation * viewTranslation;
}
