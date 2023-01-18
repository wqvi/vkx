#include <vkx/renderer/pipeline/compute.hpp>

vkx::pipeline::ComputePipeline::ComputePipeline(vk::Device logicalDevice,
						const vkx::pipeline::ComputePipelineInformation& info)
    : VulkanPipeline(logicalDevice, info.bindings) {
	const auto compShaderModule = createShaderModule(info.computeFile);

	const vk::PipelineShaderStageCreateInfo compShaderStageCreateInfo{
	    {},
	    vk::ShaderStageFlagBits::eCompute,
	    *compShaderModule,
	    "main"};

	const vk::ComputePipelineCreateInfo computePipelineCreateInfo{
	    {},
	    compShaderStageCreateInfo,
		*pipelineLayout};

	pipeline = logicalDevice.createComputePipelineUnique({}, computePipelineCreateInfo).value;
}
