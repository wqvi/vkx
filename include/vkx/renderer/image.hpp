#pragma once

#include <vkx/renderer/core/allocator.hpp>
#include <vkx/renderer/core/commands.hpp>

namespace vkx {
class Image {
public:
	VkImage resourceImage{};
	VmaAllocation resourceAllocation{};

	Image() = default;

	explicit Image(const std::string& file, const Allocator& allocator, const vkx::CommandSubmitter& commandSubmitter);

	explicit Image(const char* file, VmaAllocator allocator, const vkx::CommandSubmitter& commandSubmitter);

	void destroy(VmaAllocator allocator) const;
};
} // namespace vkx