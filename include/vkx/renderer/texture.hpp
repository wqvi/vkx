#pragma once

#include <vkx/renderer/renderer.hpp>

namespace vkx {
class Texture {
public:
	Texture() = default;

	explicit Texture(const std::string& file,
			 const vkx::VulkanDevice& device,
			 const vkx::VulkanAllocator& allocator,
			 const vkx::CommandSubmitter& commandSubmitter);

	[[nodiscard]] vk::DescriptorImageInfo createDescriptorImageInfo() const;

	[[nodiscard]] const vk::DescriptorImageInfo* getInfo() const;

private:
	vkx::Image image;
	vk::UniqueImageView view;
	vk::UniqueSampler sampler;
	vk::DescriptorImageInfo info{};
};
} // namespace vkx