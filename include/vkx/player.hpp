#pragma once

namespace vkx {
struct Entity {
	glm::vec2 position;
	glm::vec2 rotation;
};

struct Player : public Entity {

};
} // namespace vkx