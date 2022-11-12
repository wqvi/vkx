#pragma once

#include <vkx/renderer/core/renderer_types.hpp>
#include <vkx/renderer/core/vertex.hpp>
#include <vkx/renderer/uniform_buffer.hpp>

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
private:
	friend class CommandSubmitter;

	VkDevice device = nullptr;
	VkDescriptorSetLayout descriptorLayout{};
	VkPipelineLayout pipelineLayout{};
	VkPipeline pipeline{};
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets{};
	std::vector<std::vector<UniformBuffer>> uniforms;

public:
	GraphicsPipeline() = default;

	GraphicsPipeline(VkDevice device, vk::RenderPass renderPass, VmaAllocator allocator, const GraphicsPipelineInformation& info);

	const std::vector<UniformBuffer>& getUniformByIndex(std::size_t i) const;

	void destroy() const;

private:
	static VkPipelineLayout createPipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout);

	static VkShaderModule createShaderModule(VkDevice device, const char* filename);

	static VkPipeline createPipeline(VkDevice device, VkRenderPass renderPass, const GraphicsPipelineInformation& info, VkPipelineLayout pipelineLayout);
};
} // namespace vkx
