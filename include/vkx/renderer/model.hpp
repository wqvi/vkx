#pragma once

#include "vkx/renderer/core/commands.hpp"
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/image.hpp>
#include <vkx/renderer/renderer.hpp>

namespace vkx {
class Texture {
public:
	explicit Texture(const std::string& file, const vkx::VulkanDevice& device, const vkx::VulkanAllocator& allocator, const vkx::CommandSubmitter& commandSubmitter);

	[[nodiscard]] vk::DescriptorImageInfo createDescriptorImageInfo() const;

	[[nodiscard]] const vk::DescriptorImageInfo* getInfo() const;

private:
	vkx::Image image;
	vk::UniqueImageView view;
	vk::UniqueSampler sampler;
	vk::DescriptorImageInfo info{};
};
} // namespace vkx
