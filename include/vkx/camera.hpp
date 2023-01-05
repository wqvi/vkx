#pragma once

namespace vkx {
[[maybe_unused]] static constexpr glm::vec3 NORTH = glm::vec3{0.0f, 0.0f, 1.0f};

[[maybe_unused]] static constexpr glm::vec3 EAST = glm::vec3{1.0f, 0.0f, 0.0f};

[[maybe_unused]] static constexpr glm::vec3 SOUTH = glm::vec3{0.0f, 0.0f, -1.0f};

[[maybe_unused]] static constexpr glm::vec3 WEST = glm::vec3{-1.0f, 0.0f, 0.0f};

[[maybe_unused]] static constexpr glm::vec3 UP = glm::vec3{0.0f, 1.0f, 0.0f};

[[maybe_unused]] static constexpr glm::vec3 DOWN = glm::vec3{0.0f, -1.0f, 0.0f};

class Camera2D {
private:
	glm::vec2 position{0, 0};

	// This is deemed unsafe however it is a useful tool
	// For calculating any sort of rotation related stuff
	union {
		glm::vec2 rotationDegrees{0, 0};
		struct {
			float pitchDegrees;
			float yawDegrees;
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

	void rotate(const glm::vec2& rotationDegrees) noexcept;

	void move(const glm::vec2& offsetPosition) noexcept;

	[[nodiscard]] glm::vec2 globalPosition() const noexcept;
};
} // namespace vkx
