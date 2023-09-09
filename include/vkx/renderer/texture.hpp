#pragma once

#include <vkx/renderer/image.hpp>

namespace vkx {
class Texture {
public:
	vkx::Image image{};
	vk::UniqueImageView view{};
	vk::UniqueSampler sampler{};
	vk::DescriptorImageInfo descriptorImageInfo{};

	Texture() = default;

	explicit Texture(const std::string& file,
			 const vkx::VulkanInstance& instance,
			 const vkx::CommandSubmitter& commandSubmitter);

	[[nodiscard]] const vk::DescriptorImageInfo* imageInfo() const noexcept;
};
} // namespace vkx
