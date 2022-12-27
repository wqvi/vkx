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

	const VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
	    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	    nullptr,
	    0,
	    1,
	    nullptr,
	    1,
	    nullptr};

	constexpr auto fill = VK_POLYGON_MODE_FILL;
	constexpr auto wireframe = VK_POLYGON_MODE_LINE;

	const VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
	    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	    nullptr,
	    0,
	    false,
	    false,
	    fill,
	    VK_CULL_MODE_BACK_BIT,
	    VK_FRONT_FACE_COUNTER_CLOCKWISE,
	    false,
	    0.0f,
	    0.0f,
	    0.0f,
	    1.0f};

	const VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
	    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	    nullptr,
	    0,
	    VK_SAMPLE_COUNT_1_BIT,
	    false};

	const VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
	    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	    nullptr,
	    0,
	    true,
	    true,
	    VK_COMPARE_OP_LESS,
	    false,
	    false};

	constexpr auto colorMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	const VkPipelineColorBlendAttachmentState colorBlendAttachmentState{
	    true,
	    VK_BLEND_FACTOR_SRC_ALPHA,
	    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	    VK_BLEND_OP_ADD,
	    VK_BLEND_FACTOR_SRC_ALPHA,
	    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	    VK_BLEND_OP_ADD,
	    colorMask};

	const float colorBlendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	const VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
	    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
	    nullptr,
	    0,
	    false,
	    VK_LOGIC_OP_COPY,
	    1,
	    &colorBlendAttachmentState,
	    {0.0f, 0.0f, 0.0f, 0.0f}};

	const VkDynamicState pipelineDynamicStates[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	const VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
	    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
	    nullptr,
	    0,
	    2,
	    pipelineDynamicStates};

	const VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
	    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	    nullptr,
	    0,
	    static_cast<std::uint32_t>(shaderStages.size()),
	    reinterpret_cast<const VkPipelineShaderStageCreateInfo*>(shaderStages.data()),
	    reinterpret_cast<const VkPipelineVertexInputStateCreateInfo*>(&vertexInputCreateInfo),
	    reinterpret_cast<const VkPipelineInputAssemblyStateCreateInfo*>(&inputAssemblyCreateInfo),
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
	const auto result = vkCreateGraphicsPipelines(device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);
	if (result == VK_PIPELINE_COMPILE_REQUIRED_EXT) {
		throw std::runtime_error("Failed to create graphics pipeline. Compile is required.");
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline. Unknown error.");
	}

	return pipeline;
}
