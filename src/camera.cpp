#include <vkx/camera.hpp>

vkx::Camera2D::Camera2D(const glm::vec2& globalPosition,
			const glm::vec2& rotation,
			const glm::vec2& mouseSensitivity)
    : globalPosition(globalPosition), rotationDegrees(rotation), mouseSensitivity(mouseSensitivity) {}

glm::mat4 vkx::Camera2D::viewMatrix() const noexcept {
	const auto viewRotation = glm::mat3_cast(glm::conjugate(yawOrientation * pitchOrientation));
	const auto viewTranslation = glm::translate(glm::mat3(1), -globalPosition);
	return viewRotation * viewTranslation;
}

void vkx::Camera2D::rotate(const glm::vec2& rotationDegrees) noexcept {
	pitchDegrees += rotationDegrees.y * mouseSensitivity.x;
	yawDegrees += rotationDegrees.x * mouseSensitivity.y;
	pitchDegrees = glm::clamp(pitchDegrees, -89.0f, 89.0f);
	yawDegrees = glm::mod(yawDegrees, 360.0f);

	const auto pitchRadians = glm::radians(pitchDegrees);
	const auto yawRadians = -glm::radians(yawDegrees);

	pitchOrientation = glm::angleAxis(pitchRadians, vkx::EAST);
	yawOrientation = glm::angleAxis(yawRadians, vkx::UP);
}
