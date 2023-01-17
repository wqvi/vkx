#include <vkx/renderer/pipeline/pipeline.hpp>

vk::UniqueShaderModule vkx::pipeline::VulkanPipeline::createShaderModule(const std::string& filename) const {
	std::ifstream file{filename, std::ios::ate | std::ios::binary};

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file.");
	}

	const auto fileSize = static_cast<std::size_t>(file.tellg());
	std::vector<char> buffer{};
	buffer.resize(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	const vk::ShaderModuleCreateInfo shaderModuleCreateInfo{
	    {},
	    static_cast<std::uint32_t>(buffer.size()),
	    reinterpret_cast<const std::uint32_t*>(buffer.data())};

	return logicalDevice.createShaderModuleUnique(shaderModuleCreateInfo);
}