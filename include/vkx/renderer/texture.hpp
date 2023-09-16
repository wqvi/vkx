#pragma once

#include <vkx/renderer/image.hpp>

namespace vkx {
class Texture {
public:
	vk::Device logicalDevice;
	vkx::Image image{};
	vk::ImageView view{};
	vk::Sampler sampler{};
	vk::DescriptorImageInfo descriptorImageInfo{};

	Texture() = default;

	explicit Texture(const std::string& file,
			 const vkx::VulkanInstance& instance,
			 const vkx::CommandSubmitter& commandSubmitter);

	void destroy();

	[[nodiscard]] const vk::DescriptorImageInfo* imageInfo() const noexcept;
};
} // namespace vkx
