#pragma once

namespace vkx {
[[maybe_unused]] static constexpr glm::vec3 NORTH = glm::vec3{0.0f, 0.0f, 1.0f};

[[maybe_unused]] static constexpr glm::vec3 EAST = glm::vec3{1.0f, 0.0f, 0.0f};

[[maybe_unused]] static constexpr glm::vec3 SOUTH = glm::vec3{0.0f, 0.0f, -1.0f};

[[maybe_unused]] static constexpr glm::vec3 WEST = glm::vec3{-1.0f, 0.0f, 0.0f};

[[maybe_unused]] static constexpr glm::vec3 UP = glm::vec3{0.0f, 1.0f, 0.0f};

[[maybe_unused]] static constexpr glm::vec3 DOWN = glm::vec3{0.0f, -1.0f, 0.0f};

struct Camera2D {
	glm::vec2 globalPosition{0, 0};

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

	Camera2D() = default;

	explicit Camera2D(const glm::vec2& globalPosition,
			  const glm::vec2& rotation,
			  const glm::vec2& mouseSensitivity);

	glm::mat4 viewMatrix() const noexcept;

	void rotate(const glm::vec2& rotationDegrees) noexcept;
};
} // namespace vkx
