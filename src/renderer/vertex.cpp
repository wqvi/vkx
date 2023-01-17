#include <vkx/renderer/vertex.hpp>

vkx::Vertex::Vertex(const glm::vec2& pos)
    : pos(pos) {}

vkx::Vertex::Vertex(const glm::vec2& pos,
		    const glm::vec2& uv)
    : pos(pos), uv(uv) {}