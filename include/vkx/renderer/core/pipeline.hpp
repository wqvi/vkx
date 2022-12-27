#pragma once

#include <vkx/renderer/core/renderer_types.hpp>
#include <vkx/renderer/core/vertex.hpp>
#include <vkx/renderer/uniform_buffer.hpp>

namespace vkx {
struct GraphicsPipelineInformation {
	const std::string vertexFile{};
	const std::string fragmentFile{};
	const std::vector<VkDescriptorSetLayoutBinding> bindings{};
	const std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
	const std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	const std::vector<std::size_t> uniformSizes{};
	const std::vector<const Texture*> textures;
};

class GraphicsPipeline {
private:
	friend class CommandSubmitter;

	vk::Device device = nullptr;
	vk::UniqueDescriptorSetLayout descriptorLayout{};
	vk::UniquePipelineLayout pipelineLayout{};
	vk::UniquePipeline pipeline{};
	vk::UniqueDescriptorPool descriptorPool;
	std::vector<vk::DescriptorSet> descriptorSets{};
	std::vector<std::vector<UniformBuffer>> uniforms;

public:
	GraphicsPipeline() = default;

	explicit GraphicsPipeline(vk::Device device, vk::RenderPass renderPass, const vkx::VulkanAllocator& allocator, const vkx::GraphicsPipelineInformation& info);

	const std::vector<UniformBuffer>& getUniformByIndex(std::size_t i) const;

	void destroy() const;

private:
	[[nodiscard]] vk::UniqueShaderModule createShaderModule(const std::string& filename) const;

	[[nodiscard]] vk::UniquePipeline createPipeline(vk::RenderPass renderPass, const GraphicsPipelineInformation& info) const;
};
} // namespace vkx
