#include "vkx/renderer/core/device.hpp"
#include <vkx/renderer/core/pipeline.hpp>
#include <vulkan/vulkan_handles.hpp>

vkx::GraphicsPipeline::GraphicsPipeline(const vkx::Device& device, const vk::Extent2D& extent, const vk::UniqueRenderPass& renderPass, const vk::UniqueDescriptorSetLayout& descriptorSetLayout) {
    layout = createPipelineLayout(device, descriptorSetLayout);

	const auto vertShaderModule = createShaderModule(device, "shader.vert.spv");
	const auto fragShaderModule = createShaderModule(device, "shader.frag.spv");

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo({}, vk::ShaderStageFlagBits::eVertex, *vertShaderModule, "main");
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo({}, vk::ShaderStageFlagBits::eFragment, *fragShaderModule, "main");

	const auto shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

	const auto bindingDescription = Vertex::getBindingDescription();
	const auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, bindingDescription, attributeDescriptions);

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList, false);

	vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f);

	vk::Rect2D scissor({0, 0}, extent);

	vk::PipelineViewportStateCreateInfo viewportState({}, viewport, scissor);

	vk::PipelineRasterizationStateCreateInfo rasterizer(
	    {},
	    false,
	    false,
	    vk::PolygonMode::eFill,
	    vk::CullModeFlagBits::eBack,
	    vk::FrontFace::eCounterClockwise,
	    false,
	    {},
	    {},
	    {},
	    1.0f);

	vk::PipelineMultisampleStateCreateInfo multisampling(
	    {},
	    vk::SampleCountFlagBits::e1,
	    false);

	vk::PipelineDepthStencilStateCreateInfo depthStencil(
	    {},
	    true,
	    true,
	    vk::CompareOp::eLess,
	    false,
	    false);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment(false);
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

	std::array blendConstants{0.0f, 0.0f, 0.0f, 0.0f};
	vk::PipelineColorBlendStateCreateInfo colorBlending(
	    {},
	    false,
	    vk::LogicOp::eCopy,
	    colorBlendAttachment,
	    blendConstants);

	vk::GraphicsPipelineCreateInfo pipelineInfo(
	    {},
	    shaderStages,
	    &vertexInputInfo,
	    &inputAssembly,
	    {},
	    &viewportState,
	    &rasterizer,
	    &multisampling,
	    &depthStencil,
	    &colorBlending,
	    {},
	    *layout,
	    *renderPass,
	    0,
	    nullptr);

	// Use Vulkan C functions to create a pipeline due to an odd difference in vulkan headers
	// Sometimes the api returns a pipeline value or sometimes returns a result value

	VkPipeline cPipeline = nullptr;
	if (vkCreateGraphicsPipelines(static_cast<vk::Device>(device), nullptr, 1, reinterpret_cast<VkGraphicsPipelineCreateInfo*>(&pipelineInfo), nullptr, &cPipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline.");
	}

	pipeline = vk::UniquePipeline(cPipeline, *device);
}

vk::UniquePipelineLayout vkx::GraphicsPipeline::createPipelineLayout(const vkx::Device& device, const vk::UniqueDescriptorSetLayout& descriptorSetLayout) {
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *descriptorSetLayout);

	return device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

vk::UniqueShaderModule vkx::GraphicsPipeline::createShaderModule(const vkx::Device& device, const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file.");
	}

	std::size_t fileSize = static_cast<std::size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	vk::ShaderModuleCreateInfo shaderCreateInfo({}, static_cast<std::uint32_t>(buffer.size()), reinterpret_cast<const std::uint32_t*>(buffer.data()));

	return device->createShaderModuleUnique(shaderCreateInfo);
}