#pragma once

#include <vkx/renderer/renderer.hpp>

namespace vkx {
class Texture {
private:
	vkx::Image image{};
	vk::UniqueImageView view{};
	vk::UniqueSampler sampler{};
	vk::DescriptorImageInfo descriptorImageInfo{};

public:
	Texture() = default;

	explicit Texture(const std::string& file,
			 const vkx::VulkanDevice& device,
			 const vkx::VulkanAllocator& allocator,
			 const vkx::CommandSubmitter& commandSubmitter);

	[[nodiscard]] const vk::DescriptorImageInfo* imageInfo() const noexcept;
};
} // namespace vkx