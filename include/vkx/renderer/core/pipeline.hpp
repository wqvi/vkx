#pragma once

#include <vkx/renderer/buffers.hpp>
#include <vkx/renderer/core/renderer_types.hpp>
#include <vkx/renderer/core/vertex.hpp>

namespace vkx {
struct GraphicsPipelineInformation {
	const std::string vertexFile{};
	const std::string fragmentFile{};
	const std::vector<vk::DescriptorSetLayoutBinding> bindings{};
	const std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};
	const std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};
	const std::vector<std::size_t> uniformSizes{};
	const std::vector<const Texture*> textures;
};

class GraphicsPipeline {
	friend class CommandSubmitter;

private:
	vk::Device device = nullptr;
	vk::UniqueDescriptorSetLayout descriptorLayout{};
	vk::UniquePipelineLayout pipelineLayout{};
	vk::UniquePipeline pipeline{};
	vk::UniqueDescriptorPool descriptorPool;
	std::vector<vk::DescriptorSet> descriptorSets{};
	std::vector<std::vector<UniformBuffer>> uniforms;

public:
	GraphicsPipeline() = default;

	explicit GraphicsPipeline(vk::Device device,
				  vk::RenderPass renderPass,
				  const vkx::VulkanAllocator& allocator,
				  const vkx::GraphicsPipelineInformation& info);

	const std::vector<UniformBuffer>& getUniformByIndex(std::size_t i) const;

private:
	[[nodiscard]] vk::UniqueShaderModule createShaderModule(const std::string& filename) const;

	[[nodiscard]] vk::UniquePipeline createPipeline(vk::RenderPass renderPass, const GraphicsPipelineInformation& info) const;
};
} // namespace vkx
