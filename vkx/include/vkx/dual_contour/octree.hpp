#pragma once

#include <vkx/dual_contour/qef.hpp>
#include <vkx/renderer/core/renderer_base.hpp>

enum class OctreeNodeType
{
	None,
	Internal,
	Psuedo,
	Leaf,
};

struct OctreeDrawInfo
{
	std::int32_t index = -1;
	std::int32_t corners = 0;
	glm::vec3 position;
	glm::vec3 averageNormal;
	svd::QefData qef;
};

class OctreeNode
{
public:
	OctreeNode()
	{
		for (std::int32_t i = 0; i < 8; i++)
		{
			children[i] = nullptr;
		}
	}

	OctreeNode(OctreeNodeType type)
		: type(type)
	{
		for (std::int32_t i = 0; i < 8; i++)
		{
			children[i] = nullptr;
		}
	}

	OctreeNodeType type = OctreeNodeType::None;
	glm::ivec3 min = glm::ivec3(0, 0, 0);
	std::int32_t size = 0;
	OctreeNode *children[8];
	OctreeDrawInfo *drawInfo = nullptr;
};

OctreeNode *BuildOctree(const glm::ivec3 &min, const int size, const float threshold);
void DestroyOctree(OctreeNode *node);
void GenerateMeshFromOctree(OctreeNode* node, std::vector<vkx::Vertex> &vertexBuffer, std::vector<std::uint32_t> &indexBuffer);
