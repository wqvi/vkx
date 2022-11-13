#pragma once

#include <vkx/renderer/core/commands.hpp>

namespace vkx {
class Image {
public:
	explicit Image(const char* file, VmaAllocator allocator, const vkx::CommandSubmitter& commandSubmitter);

	inline VkImageView createTextureImageView(VkDevice device) const {
		return vkx::createTextureImageView(device, resourceImage);
	}

	void destroy() const;

private:
	VkImage resourceImage = nullptr;
	VmaAllocation resourceAllocation = nullptr;
	VmaAllocator allocator = nullptr;
};
} // namespace vkx