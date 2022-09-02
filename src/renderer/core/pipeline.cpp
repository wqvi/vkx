#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/core/device.hpp>

vkx::GraphicsPipeline::GraphicsPipeline(vk::Device device, const GraphicsPipelineInformation& info)
    : device(device),
      pipelineLayout(createPipelineLayout(device, info.descriptorSetLayout)),
      pipeline(createPipeline(device, info, *pipelineLayout)),
      descriptorPool(createDescriptorPool(device, info.poolSizes)),
      descriptorSets(createDescriptorSets(device, info.descriptorSetLayout, *descriptorPool)) {}

void vkx::GraphicsPipeline::updateDescriptorSets(DescriptorWriteFunction function) const {
	for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		auto descriptorInfos = function(i);
		std::vector<vk::WriteDescriptorSet> writes;
		for (std::uint32_t j = 0; j < descriptorInfos.size(); j++) {
			const auto& info = descriptorInfos[j];

			vk::WriteDescriptorSet write{};
			if (info.index() == 0) {
				// vk::DescriptorBufferInfo
				const auto* ptr = &std::get<vk::DescriptorBufferInfo>(info);
				write = {descriptorSets[i],
					 j,
					 0,
					 1,
					 vk::DescriptorType::eUniformBuffer,
					 nullptr,
					 ptr};
			} else {
				// vk::DescriptorImageInfo
				const auto* ptr = &std::get<vk::DescriptorImageInfo>(info);
				write = {descriptorSets[i],
					 j,
					 0,
					 1,
					 vk::DescriptorType::eCombinedImageSampler,
					 ptr,
					 nullptr};
			}

			writes.push_back(write);
		}
		device.updateDescriptorSets(writes, {});
	}
}

vk::UniquePipelineLayout vkx::GraphicsPipeline::createPipelineLayout(vk::Device device, vk::DescriptorSetLayout descriptorSetLayout) {
	const vk::PipelineLayoutCreateInfo pipelineLayoutInfo{{}, descriptorSetLayout};

	return device.createPipelineLayoutUnique(pipelineLayoutInfo);
}

vk::UniqueShaderModule vkx::GraphicsPipeline::createShaderModule(vk::Device device, const std::string& filename) {
	std::ifstream file{filename, std::ios::ate | std::ios::binary};

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file.");
	}

	const std::size_t fileSize = static_cast<std::size_t>(file.tellg());
	std::vector<char> buffer;
	buffer.resize(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	const vk::ShaderModuleCreateInfo shaderCreateInfo{{}, static_cast<std::uint32_t>(buffer.size()), reinterpret_cast<const std::uint32_t*>(buffer.data())};

	return device.createShaderModuleUnique(shaderCreateInfo);
}

vk::UniquePipeline vkx::GraphicsPipeline::createPipeline(vk::Device device, const GraphicsPipelineInformation& info, vk::PipelineLayout pipelineLayout) {
	const auto vertShaderModule = createShaderModule(device, info.vertexFile);
	const auto fragShaderModule = createShaderModule(device, info.fragmentFile);

	const vk::PipelineShaderStageCreateInfo vertShaderStageInfo{{}, vk::ShaderStageFlagBits::eVertex, *vertShaderModule, "main"};
	const vk::PipelineShaderStageCreateInfo fragShaderStageInfo{{}, vk::ShaderStageFlagBits::eFragment, *fragShaderModule, "main"};

	const auto shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

	const vk::PipelineVertexInputStateCreateInfo vertexInputInfo{{}, info.bindingDescriptions, info.attributeDescriptions};

	constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssembly{{}, vk::PrimitiveTopology::eTriangleList, false};

	const vk::PipelineViewportStateCreateInfo viewportState{{}, 1, nullptr, 1, nullptr};

	constexpr vk::PipelineRasterizationStateCreateInfo rasterizer{
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
	    1.0f};

	constexpr vk::PipelineMultisampleStateCreateInfo multisampling{
	    {},
	    vk::SampleCountFlagBits::e1,
	    false};

	constexpr vk::PipelineDepthStencilStateCreateInfo depthStencil{
	    {},
	    true,
	    true,
	    vk::CompareOp::eLess,
	    false,
	    false};

	constexpr auto mask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	constexpr vk::PipelineColorBlendAttachmentState colorBlendAttachment{
	    true,
	    vk::BlendFactor::eSrcAlpha,
	    vk::BlendFactor::eOneMinusSrcAlpha,
	    vk::BlendOp::eAdd,
	    vk::BlendFactor::eSrcAlpha,
	    vk::BlendFactor::eOneMinusSrcAlpha,
	    vk::BlendOp::eAdd,
	    mask};

	constexpr std::array blendConstants{0.0f, 0.0f, 0.0f, 0.0f};
	const vk::PipelineColorBlendStateCreateInfo colorBlending{
	    {},
	    false,
	    vk::LogicOp::eCopy,
	    colorBlendAttachment,
	    blendConstants};

	constexpr std::array dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

	const vk::PipelineDynamicStateCreateInfo dynamicStateInfo{{}, dynamicStates};

	const vk::GraphicsPipelineCreateInfo pipelineInfo{
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
	    &dynamicStateInfo,
	    pipelineLayout,
	    info.renderPass,
	    0,
	    nullptr};

	// Use Vulkan C api to create pipeline as the C++ bindings returns an array

	VkPipeline cPipeline = nullptr;
	const auto result = vkCreateGraphicsPipelines(device, nullptr, 1, reinterpret_cast<const VkGraphicsPipelineCreateInfo*>(&pipelineInfo), nullptr, &cPipeline);
	if (result == VK_PIPELINE_COMPILE_REQUIRED_EXT) {
		throw std::runtime_error("Failed to create graphics pipeline. Compile is required.");
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline. Unknown error.");
	}

	return vk::UniquePipeline{cPipeline, device};
}

vk::UniqueDescriptorPool vkx::GraphicsPipeline::createDescriptorPool(vk::Device device, const std::vector<vk::DescriptorPoolSize>& poolSizes) {
	const vk::DescriptorPoolCreateInfo poolInfo{{}, MAX_FRAMES_IN_FLIGHT, poolSizes};

	return device.createDescriptorPoolUnique(poolInfo);
}

std::vector<vk::DescriptorSet> vkx::GraphicsPipeline::createDescriptorSets(vk::Device device, vk::DescriptorSetLayout descriptorSetLayout, vk::DescriptorPool descriptorPool) {
	const std::vector<vk::DescriptorSetLayout> layouts{MAX_FRAMES_IN_FLIGHT, descriptorSetLayout};
	const vk::DescriptorSetAllocateInfo allocInfo{descriptorPool, layouts};

	return device.allocateDescriptorSets(allocInfo);
}