#pragma once

namespace vkx {
[[maybe_unused]] static constexpr glm::vec3 NORTH = glm::vec3{0.0f, 0.0f, 1.0f};

[[maybe_unused]] static constexpr glm::vec3 EAST = glm::vec3{1.0f, 0.0f, 0.0f};

[[maybe_unused]] static constexpr glm::vec3 SOUTH = glm::vec3{0.0f, 0.0f, -1.0f};

[[maybe_unused]] static constexpr glm::vec3 WEST = glm::vec3{-1.0f, 0.0f, 0.0f};

[[maybe_unused]] static constexpr glm::vec3 UP = glm::vec3{0.0f, 1.0f, 0.0f};

[[maybe_unused]] static constexpr glm::vec3 DOWN = glm::vec3{0.0f, -1.0f, 0.0f};

class Camera {
public:
	Camera() = default;

	explicit Camera(const glm::vec3& position);

	template <class... T>
	explicit Camera(T... t)
	    : position(t...) {}

	void updateMouse(const glm::vec2& relative);

	void updateKey(SDL_Keycode key);

	[[nodiscard]] glm::mat4 viewMatrix() const;

	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::quat yawOrientation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	glm::quat pitchOrientation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec2 sensitivity = glm::vec2(0.1f, 0.1f);
	union {
		glm::vec2 rotation = glm::vec2(0.0f, 0.0f);
		struct {
			float pitch;
			float yaw;
		};
	};
	glm::vec3 front = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
};

class Camera2D {
private:
	glm::vec2 position{0, 0};

	union {
		glm::vec2 rotation{0, 0};
		struct {
			float pitch;
			float yaw;
		};
	};

	glm::vec2 mouseSensitivity{0.5f, 0.5f};

	glm::quat yawOrientation{0.0f, 0.0f, 0.0f, 1.0f};
	glm::quat pitchOrientation{0.0f, 0.0f, 0.0f, 1.0f};

public:
	Camera2D() = default;

	explicit Camera2D(const glm::vec2& position,
			  const glm::vec2& rotation,
			  const glm::vec2& mouseSensitivity);

	glm::mat4 viewMatrix() const noexcept;

	void rotate(const glm::vec2& rotation) noexcept;

	void move(const glm::vec2& offsetPosition) noexcept;
};
} // namespace vkx
