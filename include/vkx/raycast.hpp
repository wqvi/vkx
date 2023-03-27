#pragma once

namespace vkx {
using RaycastPredicate = std::function<bool(const glm::ivec3&)>;

struct RaycastResult {
	bool success = false;
	glm::ivec3 hitPos = glm::ivec3{0};
	glm::ivec3 previousHitPos = glm::ivec3{0};
	float length = 0.0f;
};

RaycastResult raycast(const glm::vec3& origin, const glm::vec3& direction, float maxLength, RaycastPredicate predicate);

struct Raycast2DResult {
	bool success = false;
	float length = 0.0f;
	glm::vec2 hitPosition{0};
	glm::vec2 previousHitPosition{0};
};

static constexpr auto derriveStep(float direction) noexcept {
	if (direction > 0) {
		return 1;
	} else if (direction < 0) {
		return -1;
	}

	return 0;

	// return direction == 0 ? 0 : (direction > 0) ? 1 : -1; cursed?
}

static constexpr glm::ivec2 derriveStep(const glm::vec2& direction) noexcept {
	return {derriveStep(direction.x),
		derriveStep(direction.y)};
}

static constexpr auto derriveDelta(int step, float direction) noexcept {
	if (step != 0) {
		return 1.0f / glm::abs(direction);
	}

	return std::numeric_limits<float>::infinity();

	// return step != 0 ? 1.0f / glm::abs(direction) : std::numeric_limits<float>::infinity(); cursed?
}

static constexpr glm::vec2 derriveDelta(const glm::ivec2& step, const glm::vec2& direction) noexcept {
	return {derriveDelta(step.x, direction.x),
		derriveDelta(step.y, direction.y)};
}

static constexpr auto derriveCross(int step, float origin, float delta) noexcept {
	if (step != 0) {
		if (step == 1) {
			return (glm::ceil(origin) - origin) * delta;
		} else {
			return (origin - glm::floor(origin)) * delta;
		}
	}

	return std::numeric_limits<float>::infinity();
}

static constexpr glm::vec2 derriveCross(const glm::ivec2& step, const glm::vec2& origin, const glm::vec2& delta) noexcept {
	return {derriveCross(step.x, origin.x, delta.x),
		derriveCross(step.y, origin.y, delta.y)};
}

template <class T>
Raycast2DResult raycast2D(const glm::vec2& origin, const glm::vec2& direction, float maxLength, T predicate) {
	const auto step = derriveStep(direction);
	const auto delta = derriveDelta(step, direction);
	auto cross = derriveCross(step, origin, delta);

	glm::vec2 hitPos = glm::floor(origin);
	glm::vec2 previousHitPos = hitPos;
	float length = 0.0f;

	do {
		previousHitPos = hitPos;

		if (cross.x < cross.y) {
			hitPos.x += step.x;

			length = cross.x;

			cross.x += delta.x;
		} else {
			hitPos.y += step.y;

			length = cross.y;

			cross.y += delta.y;
		}

		if (length > maxLength) {
			return {false, length, glm::vec2{0}, glm::vec2{0}};
		}
	} while (!predicate(hitPos));

	return {true, length, glm::vec2(hitPos), glm::vec2(previousHitPos)};
}
} // namespace vkx