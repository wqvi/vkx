#pragma once

#include <vkx/renderer/core/renderer_types.hpp>
#include <vkx/renderer/core/vertex.hpp>
#include <vkx/renderer/uniform_buffer.hpp>

namespace vkx {
struct GraphicsPipelineInformation {
	std::string vertexFile{};
	std::string fragmentFile{};
	std::vector<vk::DescriptorSetLayoutBinding> bindings{};
	std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};
};

using DescriptorWriteFunction = std::function<std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>>(std::size_t)>;

class GraphicsPipeline {
private:
	friend class CommandSubmitter;

	vk::Device device{};
	vk::UniqueDescriptorSetLayout descriptorLayout{};
	vk::UniquePipelineLayout pipelineLayout{};
	vk::UniquePipeline pipeline{};
	vk::UniqueDescriptorPool descriptorPool;
	std::vector<vk::DescriptorSet> descriptorSets{};
	std::vector<std::vector<UniformBuffer>> uniforms;

public:
	GraphicsPipeline() = default;

	explicit GraphicsPipeline(vk::Device device, vk::RenderPass renderPass, const GraphicsPipelineInformation& info);

	void updateDescriptorSets(DescriptorWriteFunction function) const;

	const std::vector<UniformBuffer>& getUniformByIndex(std::size_t i) const;

private:
	static vk::UniquePipelineLayout createPipelineLayout(vk::Device device, vk::DescriptorSetLayout descriptorSetLayout);

	static vk::UniqueShaderModule createShaderModule(vk::Device device, const std::string& filename);

	static vk::UniquePipeline createPipeline(vk::Device device, vk::RenderPass renderPass, const GraphicsPipelineInformation& info, vk::PipelineLayout pipelineLayout);

	static vk::UniqueDescriptorPool createDescriptorPool(vk::Device device, const std::vector<vk::DescriptorPoolSize>& poolSizes);

	static std::vector<vk::DescriptorSet> createDescriptorSets(vk::Device device, vk::DescriptorSetLayout descriptorSetLayout, vk::DescriptorPool descriptorPool);
};
} // namespace vkx
