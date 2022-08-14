#include <vkx/renderer/core/pipeline.hpp>

vkx::GraphicsPipeline::GraphicsPipeline(vk::Device device, const vk::Extent2D& extent, vk::RenderPass renderPass, vk::DescriptorSetLayout descriptorSetLayout) {
	layout = createPipelineLayout(device, descriptorSetLayout);
	pipeline = createPipeline(device, extent, renderPass, *layout);

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully created renderer graphics pipeline.");
}

vk::UniquePipelineLayout vkx::GraphicsPipeline::createPipelineLayout(vk::Device device, vk::DescriptorSetLayout descriptorSetLayout) {
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, descriptorSetLayout);

	return device.createPipelineLayoutUnique(pipelineLayoutInfo);
}

vk::UniqueShaderModule vkx::GraphicsPipeline::createShaderModule(vk::Device device, const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file.");
	}

	std::size_t fileSize = static_cast<std::size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	vk::ShaderModuleCreateInfo shaderCreateInfo({}, static_cast<std::uint32_t>(buffer.size()), reinterpret_cast<const std::uint32_t*>(buffer.data()));

	return device.createShaderModuleUnique(shaderCreateInfo);
}

vk::UniquePipeline vkx::GraphicsPipeline::createPipeline(vk::Device device, const vk::Extent2D& extent, vk::RenderPass renderPass, vk::PipelineLayout layout) {
	const auto vertShaderModule = createShaderModule(device, "shader.vert.spv");
	const auto fragShaderModule = createShaderModule(device, "shader.frag.spv");

	const vk::PipelineShaderStageCreateInfo vertShaderStageInfo({}, vk::ShaderStageFlagBits::eVertex, *vertShaderModule, "main");
	const vk::PipelineShaderStageCreateInfo fragShaderStageInfo({}, vk::ShaderStageFlagBits::eFragment, *fragShaderModule, "main");

	const auto shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

	constexpr auto bindingDescription = Vertex::getBindingDescription();
	constexpr auto attributeDescriptions = Vertex::getAttributeDescriptions();

	const vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, bindingDescription, attributeDescriptions);

	constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList, false);

	const vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f);

	const vk::Rect2D scissor({0, 0}, extent);

	const vk::PipelineViewportStateCreateInfo viewportState({}, viewport, scissor);

	constexpr vk::PipelineRasterizationStateCreateInfo rasterizer(
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

	constexpr vk::PipelineMultisampleStateCreateInfo multisampling(
	    {},
	    vk::SampleCountFlagBits::e1,
	    false);

	constexpr vk::PipelineDepthStencilStateCreateInfo depthStencil(
	    {},
	    true,
	    true,
	    vk::CompareOp::eLess,
	    false,
	    false);

	constexpr auto mask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	constexpr vk::PipelineColorBlendAttachmentState colorBlendAttachment(
	    false,
	    vk::BlendFactor::eZero,
	    vk::BlendFactor::eZero,
	    vk::BlendOp::eAdd,
	    vk::BlendFactor::eZero,
	    vk::BlendFactor::eZero,
	    vk::BlendOp::eAdd,
	    mask);

	constexpr std::array blendConstants{0.0f, 0.0f, 0.0f, 0.0f};
	const vk::PipelineColorBlendStateCreateInfo colorBlending(
	    {},
	    false,
	    vk::LogicOp::eCopy,
	    colorBlendAttachment,
	    blendConstants);

	const vk::GraphicsPipelineCreateInfo pipelineInfo(
	    {},
	    shaderStages,
	    &vertexInputInfo,
	    &inputAssembly,
	    nullptr,
	    &viewportState,
	    &rasterizer,
	    &multisampling,
	    &depthStencil,
	    &colorBlending,
	    nullptr,
	    layout,
	    renderPass,
	    0,
	    nullptr);

	// Use Vulkan C api to create pipeline as the C++ bindings returns an array

	VkPipeline cPipeline = nullptr;
	const auto result = vkCreateGraphicsPipelines(device, nullptr, 1, reinterpret_cast<const VkGraphicsPipelineCreateInfo*>(&pipelineInfo), nullptr, &cPipeline);
	if (result == VK_PIPELINE_COMPILE_REQUIRED_EXT) {
		throw std::runtime_error("Failed to create graphics pipeline. Compile is required.");
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline. Unknown error.");
	}

	return vk::UniquePipeline(cPipeline, device);
}