#pragma once

constexpr static const glm::vec3 NORTH(0.0f, 0.0f, 1.0f);
constexpr static const glm::vec3 EAST(1.0f, 0.0f, 0.0f);
constexpr static const glm::vec3 SOUTH(0.0f, 0.0f, -1.0f);
constexpr static const glm::vec3 WEST(-1.0f, 0.0f, 0.0f);
constexpr static const glm::vec3 UP(0.0f, 1.0f, 0.0f);
constexpr static const glm::vec3 DOWN(0.0f, -1.0f, 0.0f);

/* TO BE FIXED BUT DON'T TOUCH FOR NOW! */
class Camera {
public:
	Camera() = default;
	explicit Camera(glm::vec3 const &position);
	void updateMouse(glm::vec2 const &relative);
	void updateKey(int key);
	[[nodiscard]] glm::mat4 viewMatrix() const;

	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::quat yawOrientation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	glm::quat pitchOrientation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec2 sensitivity = glm::vec2(0.1f, 0.1f);
	glm::vec2 rotation = glm::vec2(0.0f, 0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
};