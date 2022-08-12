#pragma once

namespace vkx {
[[maybe_unused]] static constexpr glm::vec3 NORTH = glm::vec3(0.0f, 0.0f, 1.0f);

[[maybe_unused]] static constexpr glm::vec3 EAST = glm::vec3(1.0f, 0.0f, 0.0f);

[[maybe_unused]] static constexpr glm::vec3 SOUTH = glm::vec3(0.0f, 0.0f, -1.0f);

[[maybe_unused]] static constexpr glm::vec3 WEST = glm::vec3(-1.0f, 0.0f, 0.0f);

[[maybe_unused]] static constexpr glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);

[[maybe_unused]] static constexpr glm::vec3 DOWN = glm::vec3(0.0f, -1.0f, 0.0f);

class Camera {
public:
	Camera() = default;

	explicit Camera(const glm::vec3& position);

	void updateMouse(const glm::vec2& relative);

	void updateKey(SDL_Keycode key);

	[[nodiscard]] glm::mat4 viewMatrix() const;

	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::quat yawOrientation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	glm::quat pitchOrientation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec2 sensitivity = glm::vec2(0.1f, 0.1f);
	glm::vec2 rotation = glm::vec2(0.0f, 0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
};
} // namespace vkx
