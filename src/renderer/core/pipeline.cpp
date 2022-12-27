#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/model.hpp>
#include <vkx/renderer/renderer.hpp>
#include <vkx/renderer/uniform_buffer.hpp>

vkx::GraphicsPipeline::GraphicsPipeline(vk::Device device, vk::RenderPass renderPass, const vkx::VulkanAllocator& allocator, const vkx::GraphicsPipelineInformation& info)
    : device(device) {
	const vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
	    {},
	    static_cast<std::uint32_t>(info.bindings.size()),
	    reinterpret_cast<const vk::DescriptorSetLayoutBinding*>(info.bindings.data())};

	descriptorLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

	const vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{{}, descriptorLayout};

	pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

	pipeline = createPipeline(device, renderPass, info, pipelineLayout);

	std::vector<vk::DescriptorPoolSize> poolSizes{};
	poolSizes.reserve(info.bindings.size());
	for (const auto& info : info.bindings) {
		poolSizes.emplace_back(static_cast<vk::DescriptorType>(info.descriptorType), vkx::MAX_FRAMES_IN_FLIGHT);
	}

	const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{{}, vkx::MAX_FRAMES_IN_FLIGHT, poolSizes};

	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);

	const std::vector layouts{vkx::MAX_FRAMES_IN_FLIGHT, descriptorLayout};

	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{descriptorPool, layouts};

	descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);

	for (std::size_t size : info.uniformSizes) {
		uniforms.push_back(vkx::allocateUniformBuffers(static_cast<VmaAllocator>(allocator), size));
	}

	for (std::uint32_t i = 0; i < vkx::MAX_FRAMES_IN_FLIGHT; i++) {
		const auto descriptorSet = descriptorSets[i];

		auto uniformsBegin = uniforms.cbegin();
		auto texturesBegin = info.textures.cbegin();

		std::vector<vk::WriteDescriptorSet> writes;

		for (std::uint32_t j = 0; j < poolSizes.size(); j++) {
			const auto type = poolSizes[j].type;

			vk::WriteDescriptorSet write{descriptorSet, j, 0, 1, type};
			if (type == vk::DescriptorType::eUniformBuffer) {
				const auto& uniform = *uniformsBegin;
				write.pBufferInfo = reinterpret_cast<const vk::DescriptorBufferInfo*>(uniform[i].getInfo());
				uniformsBegin++;
			} else if (type == vk::DescriptorType::eCombinedImageSampler) {
				const auto& texture = *texturesBegin;
				write.pImageInfo = reinterpret_cast<const vk::DescriptorImageInfo*>(texture->getInfo());
				texturesBegin++;
			}

			writes.push_back(write);
		}

		device.updateDescriptorSets(writes, {});
	}
}

const std::vector<vkx::UniformBuffer>& vkx::GraphicsPipeline::getUniformByIndex(std::size_t i) const {
	return uniforms[i];
}

void vkx::GraphicsPipeline::destroy() const {
	for (const auto& uniformGroup : uniforms) {
		for (const auto& uniform : uniformGroup) {
			uniform.destroy();
		}
	}

	vkDestroyDescriptorSetLayout(device, descriptorLayout, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

vk::UniqueShaderModule vkx::GraphicsPipeline::createShaderModule(vk::Device device, const char* filename) {
	std::ifstream file{filename, std::ios::ate | std::ios::binary};

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file.");
	}

	const std::size_t fileSize = static_cast<std::size_t>(file.tellg());
	std::vector<char> buffer;
	buffer.resize(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	const vk::ShaderModuleCreateInfo shaderModuleCreateInfo{{}, static_cast<std::uint32_t>(buffer.size()), reinterpret_cast<const std::uint32_t*>(buffer.data())};

	return device.createShaderModuleUnique(shaderModuleCreateInfo);
}

VkPipeline vkx::GraphicsPipeline::createPipeline(VkDevice device, VkRenderPass renderPass, const GraphicsPipelineInformation& info, VkPipelineLayout pipelineLayout) {
	const auto vertShaderModule = createShaderModule(device, info.vertexFile.c_str());
	const auto fragShaderModule = createShaderModule(device, info.fragmentFile.c_str());

	const vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo{
	    {},
	    vk::ShaderStageFlagBits::eVertex,
	    *vertShaderModule,
	    "main"};

	const vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo{
	    {},
	    vk::ShaderStageFlagBits::eFragment,
	    *fragShaderModule,
	    "main"};

	const std::vector shaderStages = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

	const vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo{
	    {},
	    static_cast<std::uint32_t>(info.bindingDescriptions.size()),
	    reinterpret_cast<const vk::VertexInputBindingDescription*>(info.bindingDescriptions.data()),
	    static_cast<std::uint32_t>(info.attributeDescriptions.size()),
	    reinterpret_cast<const vk::VertexInputAttributeDescription*>(info.attributeDescriptions.data())};

	const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{
	    {},
	    vk::PrimitiveTopology::eTriangleList,
	    false};

	const vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{{}, 1, nullptr, 1, nullptr};

	constexpr auto fill = vk::PolygonMode::eFill;
	constexpr auto wireframe = vk::PolygonMode::eLine;

	const vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
	    {},
	    false,
	    false,
	    fill,
	    vk::CullModeFlagBits::eBack,
	    vk::FrontFace::eCounterClockwise,
	    false,
	    0.0f,
	    0.0f,
	    0.0f,
	    1.0f};

	const vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
	    {},
	    vk::SampleCountFlagBits::e1,
	    false};

	const vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
	    {},
	    true,
	    true,
	    vk::CompareOp::eLess,
	    false,
	    false};

	using Color = vk::ColorComponentFlagBits;
	using Factor = vk::BlendFactor;
	using Op = vk::BlendOp;

	constexpr auto colorMask = Color::eR | Color::eG | Color::eB | Color::eA;

	const vk::PipelineColorBlendAttachmentState colorBlendAttachmentState{
	    true,
	    Factor::eSrcAlpha,
	    Factor::eOneMinusSrcAlpha,
	    Op::eAdd,
	    Factor::eSrcAlpha,
	    Factor::eOneMinusSrcAlpha,
	    Op::eAdd,
	    colorMask};

	const vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
	    {},
	    false,
	    vk::LogicOp::eCopy,
	    colorBlendAttachmentState,
	    {0.0f, 0.0f, 0.0f, 0.0f}};

	constexpr std::array pipelineDynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

	const vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{{}, pipelineDynamicStates};

	const vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
	    {},
	    static_cast<std::uint32_t>(shaderStages.size()),
	    shaderStages.data(),
	    &vertexInputCreateInfo,
	    &inputAssemblyCreateInfo,
	    nullptr,
	    &viewportStateCreateInfo,
	    &rasterizationStateCreateInfo,
	    &multisampleStateCreateInfo,
	    &depthStencilStateCreateInfo,
	    &colorBlendStateCreateInfo,
	    &dynamicStateCreateInfo,
	    pipelineLayout,
	    renderPass,
	    0,
	    nullptr};

	VkPipeline pipeline = nullptr;
	const auto result = vkCreateGraphicsPipelines(device, nullptr, 1, reinterpret_cast<const VkGraphicsPipelineCreateInfo*>(&graphicsPipelineCreateInfo), nullptr, &pipeline);
	if (result == VK_PIPELINE_COMPILE_REQUIRED_EXT) {
		throw std::runtime_error("Failed to create graphics pipeline. Compile is required.");
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline. Unknown error.");
	}

	return pipeline;
}
