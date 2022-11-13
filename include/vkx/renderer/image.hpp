#pragma once

#include <vkx/renderer/core/allocator.hpp>
#include <vkx/renderer/core/commands.hpp>

namespace vkx {
class Image {
public:
	VkImage resourceImage = nullptr;
	VmaAllocation resourceAllocation = nullptr;

	Image() = default;

	explicit Image(const std::string& file, const Allocator& allocator, const vkx::CommandSubmitter& commandSubmitter);

	explicit Image(const char* file, VmaAllocator allocator, const vkx::CommandSubmitter& commandSubmitter);

	void destroy() const;

private:
	VmaAllocator allocator = nullptr;
};
} // namespace vkx