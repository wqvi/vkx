#pragma once

#include <vkx/renderer/core/allocator.hpp>
#include <vkx/renderer/core/commands.hpp>

namespace vkx {
class Image {
public:
	std::shared_ptr<Allocation<vk::Image>> resource;

	Image() = default;

	explicit Image(const std::string& file, const Allocator& allocator, const std::shared_ptr<vkx::CommandSubmitter>& commandSubmitter);
};
} // namespace vkx