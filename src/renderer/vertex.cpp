#include <vkx/renderer/vertex.hpp>

vkx::Vertex::Vertex(const glm::vec2& pos,
		    const glm::vec2& uv,
		    const glm::vec2& normal)
    : pos(pos), uv(uv), normal(normal) {}