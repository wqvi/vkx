#pragma once

#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/renderer.hpp>

namespace vkx {
class Image {
public:
	explicit Image(const char* file, VmaAllocator allocator, const vkx::CommandSubmitter& commandSubmitter);

	inline VkImageView createTextureImageView(VkDevice device) const {
		return vkx::createImageView(device, resourceImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void destroy() const;

private:
	VkImage resourceImage = nullptr;
	VmaAllocation resourceAllocation = nullptr;
	VmaAllocator allocator = nullptr;
};
} // namespace vkx