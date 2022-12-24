#pragma once

#include "vkx/renderer/core/commands.hpp"
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/image.hpp>
#include <vkx/renderer/renderer.hpp>

namespace vkx {
class Texture {
public:
	explicit Texture(const char* file, VkDevice device, float maxAnisotropy, const vkx::VulkanAllocator& allocator, const vkx::CommandSubmitter& commandSubmitter);

	[[nodiscard]] VkDescriptorImageInfo createDescriptorImageInfo() const;

	[[nodiscard]] const VkDescriptorImageInfo* getInfo() const;

	void destroy() const;

private:
	Image image;
	VkImageView view = nullptr;
	VkSampler sampler = nullptr;
	VkDescriptorImageInfo info{};
	VkDevice device = nullptr;
};
} // namespace vkx
