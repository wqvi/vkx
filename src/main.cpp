#include "vkx/renderer/core/renderer_base.hpp"
#include "vkx/renderer/core/renderer_types.hpp"
#include "vkx/renderer/core/vertex.hpp"
#include "vkx/renderer/uniform_buffer.hpp"
#include <SDL2/SDL_log.h>
#include <cstddef>
#include <cstdint>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <iostream>
#include <stdexcept>
#include <vk_mem_alloc.h>
#include <vkx/vkx.hpp>
#include <vulkan/vulkan_core.h>

template <class T>
struct UniformVariable {
	T variable;
	VmaAllocation allocation;
};

int main(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failure to initialize SDL2: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	auto windowFlags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;

	SDL_Window* window = SDL_CreateWindow("Jewelry", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, windowFlags);

	if (window == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failure to initialize SDL2 window: %s", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	{
		VulkanBootstrap bootstrap{window};
		VulkanDevice device = bootstrap.createDevice();
		VulkanSwapchain swapchain = device.createSwapchain(window);
		VkRenderPass clearRenderPass = device.createRenderPass(swapchain.getImageFormat(), VK_ATTACHMENT_LOAD_OP_CLEAR);
		swapchain.createFramebuffers(static_cast<VkDevice>(device), clearRenderPass);
		VkDescriptorSetLayoutBinding uboLayoutBinding = {
		    .binding = 0,
		    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		    .descriptorCount = 1,
		    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		    .pImmutableSamplers = nullptr};

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {
		    .binding = 1,
		    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		    .descriptorCount = 1,
		    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		    .pImmutableSamplers = nullptr};

		std::vector<VkDescriptorSetLayoutBinding> bindings{uboLayoutBinding, samplerLayoutBinding};

		VkDescriptorSetLayout descriptorSetLayout = device.createDescriptorSetLayout(bindings);

		GraphicsPipelineInfo info = {
		    .vertexFile = "shader2D.vert.spv",
		    .fragmentFile = "shader2D.frag.spv",
		    .extent = swapchain.getExtent(),
		    .renderPass = clearRenderPass,
		    .descriptorSetLayout = descriptorSetLayout};

		VulkanGraphicsPipeline graphicsPipeline = device.createGraphicsPipeline(info);

		const auto syncObjects = SyncObjects::createSyncObjects(static_cast<VkDevice>(device));

		const auto drawCommands = device.createDrawCommands(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolSize uniformBufferDescriptor = {
		    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		    .descriptorCount = MAX_FRAMES_IN_FLIGHT};

		VkDescriptorPoolSize textureDescriptor = {
		    .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		    .descriptorCount = MAX_FRAMES_IN_FLIGHT};

		std::array<VkDescriptorPoolSize, 2> descriptors{uniformBufferDescriptor, textureDescriptor};

		VkDescriptorPoolCreateInfo poolCreateInfo = {
		    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		    .pNext = nullptr,
		    .flags = 0,
		    .maxSets = MAX_FRAMES_IN_FLIGHT,
		    .poolSizeCount = static_cast<std::uint32_t>(descriptors.size()),
		    .pPoolSizes = descriptors.data()};

		VkDescriptorPool descriptorPool = nullptr;
		if (vkCreateDescriptorPool(static_cast<VkDevice>(device), &poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor pool.");
		}

		/*
		std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
						     *descriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo{*descriptorPool, layouts};

	descriptorSets = (*device)->allocateDescriptorSets(allocInfo);

	for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		std::array<vk::WriteDescriptorSet, 4> descriptorWrites{
		    mvpBuffers[i].createWriteDescriptorSet(descriptorSets[i], 0),
		    texture.createWriteDescriptorSet(descriptorSets[i], 1),
		    lightBuffers[i].createWriteDescriptorSet(descriptorSets[i], 2),
		    materialBuffers[i].createWriteDescriptorSet(descriptorSets[i], 3),
		};

		(*device)->updateDescriptorSets(descriptorWrites, {});
	}
		*/

		std::vector<UniformVariable<VkBuffer>> uboBuffers;
		uboBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			uboBuffers[i].allocation = device.allocateBuffer(sizeof(glm::mat4) * 3, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &uboBuffers[i].variable);
		}

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		    .pNext = nullptr,
		    .descriptorPool = descriptorPool,
		    .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
		    .pSetLayouts = descriptorSetLayouts.data()};

		std::vector<VkDescriptorSet> descriptorSets;
		descriptorSets.reserve(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(static_cast<VkDevice>(device), &descriptorSetAllocateInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor sets.");
		}

		for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo = {
				.buffer = uboBuffers[i].variable,
				.offset = 0,
				.range = sizeof(glm::mat4) * 3
			};

			VkWriteDescriptorSet uniformWrite = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = descriptorSets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pImageInfo = nullptr,
				.pBufferInfo = &bufferInfo,
				.pTexelBufferView = nullptr
			};

			// VkDescriptorImageInfo imageInfo = {
			// 	.sampler = nullptr,
			// 	.imageView = nullptr,
			// 	.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			// };

			// VkWriteDescriptorSet textureWrite = {
			// 	.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			// 	.pNext = nullptr,
			// 	.dstSet = descriptorSets[i],
			// 	.dstBinding = 1,
			// 	.dstArrayElement = 0,
			// 	.descriptorCount = 1,
			// 	.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			// 	.pImageInfo = &imageInfo,
			// 	.pBufferInfo = nullptr,
			// 	.pTexelBufferView = nullptr
			// };

			std::array<VkWriteDescriptorSet, 1> writeDescriptorSets {uniformWrite};

			vkUpdateDescriptorSets(static_cast<VkDevice>(device), static_cast<std::uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}

		vkx::RendererBase renderer{window};

		vkx::Model model{};

		std::vector<vkx::Vertex> vertices{
		    {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
		    {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
		    {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
		    {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}};

		std::vector<std::uint32_t> indices{
		    0, 1, 2, 2, 3, 0};

		std::vector<Vertex> testVertices{
		    {{0.0f, 0.0f}},
		    {{1.0f, 0.0f}},
		    {{1.0f, 1.0f}},
		    {{0.0f, 1.0f}}};

		std::vector<std::uint32_t> testIndices{
		    0, 1, 2, 2, 3, 0};

		model = vkx::Model{renderer.allocateMesh(vertices, indices),
				   renderer.allocateTexture("a.jpg"),
				   {glm::vec3(0.2f), 100.0f}};

		std::vector<vkx::UniformBuffer<vkx::MVP>> mvpBuffers;
		std::vector<vkx::UniformBuffer<vkx::DirectionalLight>> lightBuffers;
		std::vector<vkx::UniformBuffer<vkx::Material>> materialBuffers;

		mvpBuffers = renderer.createBuffers(vkx::MVP{});
		lightBuffers = renderer.createBuffers(vkx::DirectionalLight{});
		materialBuffers = renderer.createBuffers(vkx::Material{});
		renderer.createDescriptorSets(mvpBuffers, lightBuffers, materialBuffers, model.texture);

		glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 100.0f));
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		glm::mat4 proj = glm::ortho(0.0f, 640.0f, 480.0f, 0.0f, 0.1f, 100.0f);

		SDL_Event event{};
		bool isRunning = true;
		SDL_ShowWindow(window);
		while (isRunning) {
			auto currentFrame = renderer.getCurrentFrameIndex();

			const auto currentIndex = swapchain.updateCurrentFrameIndex();

			vkWaitForFences(static_cast<VkDevice>(device), 1, &syncObjects[currentIndex].inFlightFence, VK_TRUE, UINT64_MAX);

			std::uint32_t imageIndex = 0;
			auto result = swapchain.acquireNextImage(static_cast<VkDevice>(device), syncObjects[currentIndex].imageAvailableSemaphore, &imageIndex);

			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				int width;
				int height;
				SDL_Vulkan_GetDrawableSize(window, &width, &height);
				while (width == 0 || height == 0) {
					SDL_Vulkan_GetDrawableSize(window, &width, &height);
					SDL_WaitEvent(nullptr);
				}
				device.waitIdle();

				swapchain.destroy();
				graphicsPipeline.destroy();

				swapchain = device.createSwapchain(window);

				clearRenderPass = device.createRenderPass(swapchain.getImageFormat(), VK_ATTACHMENT_LOAD_OP_CLEAR);

				swapchain.createFramebuffers(static_cast<VkDevice>(device), clearRenderPass);

				info.extent = swapchain.getExtent();
				graphicsPipeline = device.createGraphicsPipeline(info);
				continue;
			} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
				throw std::runtime_error("Vulkan swapchain failed to acquire next image.");
			}

			device.mapMemory(uboBuffers[currentIndex].allocation, nullptr);

			vkResetFences(static_cast<VkDevice>(device), 1, &syncObjects[currentIndex].inFlightFence);

			/*
			mvpBuffer.mapMemory();
	lightBuffer.mapMemory();
	materialBuffer.mapMemory();

	(*device)->resetFences(*syncObjects[currentIndexFrame].inFlightFence);

	drawCommands[currentIndexFrame].record(
	    *renderPass, *swapchain.framebuffers[imageIndex], swapchain.extent,
	    *graphicsPipeline.pipeline, *graphicsPipeline.layout,
	    descriptorSets[currentIndexFrame], vertexBuffer, indexBuffer, indexCount);

	std::vector<vk::CommandBuffer> commandBuffers{
	    static_cast<vk::CommandBuffer>(drawCommands[currentIndexFrame])};
	device->submit(commandBuffers,
		       *syncObjects[currentIndexFrame].imageAvailableSemaphore,
		       *syncObjects[currentIndexFrame].renderFinishedSemaphore,
		       *syncObjects[currentIndexFrame].inFlightFence);

	result =
	    device->present(swapchain, imageIndex,
			    *syncObjects[currentIndexFrame].renderFinishedSemaphore);

	if (result == vk::Result::eErrorOutOfDateKHR ||
	    result == vk::Result::eSuboptimalKHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapchain();
	} else if (result != vk::Result::eSuccess) {
		throw vkx::VulkanError(result);
	}

	currentIndexFrame = (currentIndexFrame + 1) % MAX_FRAMES_IN_FLIGHT;
			*/

			auto& mvpBuffer = mvpBuffers[currentFrame];
			mvpBuffer->model = modelMatrix;
			mvpBuffer->view = view;
			mvpBuffer->proj = proj;

			auto& lightBuffer = lightBuffers[currentFrame];

			auto& materialBuffer = materialBuffers[currentFrame];

			// TODO whatever this is lol
			renderer.drawFrame(mvpBuffer, lightBuffer, materialBuffer,
					   model.mesh.vertexBuffer, model.mesh.indexBuffer,
					   static_cast<std::uint32_t>(model.mesh.indexCount),
					   currentFrame);

			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_QUIT:
					isRunning = false;
					break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
						renderer.framebufferResized = true;
						proj = glm::ortho(0.0f, static_cast<float>(event.window.data1), static_cast<float>(event.window.data2), 0.0f, 0.1f, 100.0f);
					}
					break;
				}
			}
		}

		for (const auto& variable : uboBuffers) {
			vmaDestroyBuffer(device.getAllocator(), variable.variable, variable.allocation);
		}

		vkDestroyDescriptorPool(static_cast<VkDevice>(device), descriptorPool, nullptr);

		for (const auto& syncObject : syncObjects) {
			syncObject.destroy(static_cast<VkDevice>(device));
		}

		graphicsPipeline.destroy();
		vkDestroyDescriptorSetLayout(static_cast<VkDevice>(device), descriptorSetLayout, nullptr);
		vkDestroyRenderPass(static_cast<VkDevice>(device), clearRenderPass, nullptr);
		swapchain.destroy();
		device.destroy();

		renderer.waitIdle();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
