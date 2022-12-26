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

	pipelineLayout = createPipelineLayout(device, descriptorLayout);

	pipeline = createPipeline(device, renderPass, info, pipelineLayout);

	std::vector<VkDescriptorPoolSize> poolSizes{};
	poolSizes.reserve(info.bindings.size());
	for (const auto& info : info.bindings) {
		poolSizes.emplace_back(VkDescriptorPoolSize{info.descriptorType, vkx::MAX_FRAMES_IN_FLIGHT});
	}

	const VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		0,
		vkx::MAX_FRAMES_IN_FLIGHT,
		static_cast<std::uint32_t>(poolSizes.size()),
		poolSizes.data()};

	if (vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline descriptor pool.");
	}

	const std::vector layouts{vkx::MAX_FRAMES_IN_FLIGHT, descriptorLayout};

	const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr,
		descriptorPool,
		static_cast<std::uint32_t>(layouts.size()),
		layouts.data()};

	descriptorSets.resize(vkx::MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate graphics pipeline descriptor sets.");
	}

	for (std::size_t size : info.uniformSizes) {
		uniforms.push_back(vkx::allocateUniformBuffers(static_cast<VmaAllocator>(allocator), size));
	}

	for (std::uint32_t i = 0; i < vkx::MAX_FRAMES_IN_FLIGHT; i++) {
		const auto descriptorSet = descriptorSets[i];

		auto uniformsBegin = uniforms.cbegin();
		auto texturesBegin = info.textures.cbegin();

		std::vector<VkWriteDescriptorSet> writes;

		for (std::uint32_t j = 0; j < poolSizes.size(); j++) {
			const auto type = poolSizes[j].type;

			VkWriteDescriptorSet write{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				descriptorSet, 
				j, 
				0, 
				1, 
				type, 
				nullptr, 
				nullptr};
			if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				const auto& uniform = *uniformsBegin;
				write.pBufferInfo = uniform[i].getInfo();
				uniformsBegin++;
			} else if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				const auto& texture = *texturesBegin;
				write.pImageInfo = texture->getInfo();
				texturesBegin++;
			}

			writes.push_back(write);
		}

		vkUpdateDescriptorSets(device, static_cast<std::uint32_t>(writes.size()), writes.data(), 0, nullptr);
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

VkPipelineLayout vkx::GraphicsPipeline::createPipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout) {
	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
	    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	    nullptr,
	    0,
	    1,
	    &descriptorSetLayout,
	    0,
	    nullptr};

	VkPipelineLayout pipelineLayout = nullptr;
	if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline layout.");
	}

	return pipelineLayout;
}

VkShaderModule vkx::GraphicsPipeline::createShaderModule(VkDevice device, const char* filename) {
	std::ifstream file{filename, std::ios::ate | std::ios::binary};

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file.");
	}

	const std::size_t fileSize = static_cast<std::size_t>(file.tellg());
	std::vector<char> buffer;
	buffer.resize(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	const VkShaderModuleCreateInfo shaderModuleCreateInfo{
	    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	    nullptr,
	    0,
	    static_cast<std::uint32_t>(buffer.size()),
	    reinterpret_cast<const std::uint32_t*>(buffer.data())};

	VkShaderModule shaderModule = nullptr;
	if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module");
	}

	return shaderModule;
}

VkPipeline vkx::GraphicsPipeline::createPipeline(VkDevice device, VkRenderPass renderPass, const GraphicsPipelineInformation& info, VkPipelineLayout pipelineLayout) {
	const auto vertShaderModule = createShaderModule(device, info.vertexFile.c_str());
	const auto fragShaderModule = createShaderModule(device, info.fragmentFile.c_str());

	const VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{
	    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	    nullptr,
	    0,
	    VK_SHADER_STAGE_VERTEX_BIT,
	    vertShaderModule,
	    "main",
	    nullptr};

	const VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{
	    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	    nullptr,
	    0,
	    VK_SHADER_STAGE_FRAGMENT_BIT,
	    fragShaderModule,
	    "main",
	    nullptr};

	const std::vector shaderStages = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

	const VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{
	    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	    nullptr,
	    0,
	    static_cast<std::uint32_t>(info.bindingDescriptions.size()),
	    reinterpret_cast<const VkVertexInputBindingDescription*>(info.bindingDescriptions.data()),
	    static_cast<std::uint32_t>(info.attributeDescriptions.size()),
	    reinterpret_cast<const VkVertexInputAttributeDescription*>(info.attributeDescriptions.data())};

	const VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{
	    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	    nullptr,
	    0,
	    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
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
	const auto result = vkCreateGraphicsPipelines(device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);
	if (result == VK_PIPELINE_COMPILE_REQUIRED_EXT) {
		throw std::runtime_error("Failed to create graphics pipeline. Compile is required.");
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline. Unknown error.");
	}

	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);

	return pipeline;
}
