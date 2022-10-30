#include "vkx/renderer/core/commands.hpp"
#include "vkx/renderer/core/pipeline.hpp"
#include "vkx/renderer/renderer.hpp"
#include <SDL2/SDL.h>
#include <stdexcept>
#include <vkx/vkx.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>

auto createShaderDescriptorSetLayout(vk::Device device) {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr};

	constexpr vk::DescriptorSetLayoutBinding samplerLayoutBinding{
	    1,
	    vk::DescriptorType::eCombinedImageSampler,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr};

	constexpr vk::DescriptorSetLayoutBinding lightLayoutBinding{
	    2,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr};

	constexpr vk::DescriptorSetLayoutBinding materialLayoutBinding{
	    3,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr};

	constexpr std::array bindings = {uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding, materialLayoutBinding};

	const vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, bindings};
	return device.createDescriptorSetLayoutUnique(layoutInfo);
}

auto createShaderBindings() {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr};

	constexpr vk::DescriptorSetLayoutBinding samplerLayoutBinding{
	    1,
	    vk::DescriptorType::eCombinedImageSampler,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr};

	constexpr vk::DescriptorSetLayoutBinding lightLayoutBinding{
	    2,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr};

	constexpr vk::DescriptorSetLayoutBinding materialLayoutBinding{
	    3,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr};

	return std::vector{uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding, materialLayoutBinding};
}

auto createHighlightDescriptorSetLayout(vk::Device device) {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr};

	constexpr std::array bindings = {uboLayoutBinding};

	const vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, bindings};
	return device.createDescriptorSetLayoutUnique(layoutInfo);
}

auto createHighlightShaderBindings() noexcept {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr};

	return std::vector{uboLayoutBinding};
}

auto getBindingDescription() noexcept {
	std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};

	bindingDescriptions.push_back({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex});

	return bindingDescriptions;
}

auto getAttributeDescriptions() noexcept {
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};

	attributeDescriptions.push_back({0, 0, vk::Format::eR32G32B32Sfloat, 0});

	return attributeDescriptions;
}

int main(void) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
		return EXIT_FAILURE;
	}

	if (SDL_SetRelativeMouseMode(SDL_TRUE)) {
		return EXIT_FAILURE;
	}

	SDL_Window* window = SDL_CreateWindow("vkx", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
	if (window == nullptr) {
		return EXIT_FAILURE;
	}

	const auto instance = vkx::createInstance(window);
	const auto surface = vkx::createSurface(window, instance);
	const auto physicalDevice = vkx::getBestPhysicalDevice(instance, surface);
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	const float maxSamplerAnisotropy = properties.limits.maxSamplerAnisotropy;
	const auto logicalDevice = vkx::createDevice(instance, surface, physicalDevice);

	{
		vkx::Camera camera({0, 0, 0});

		const vkx::Allocator allocator{physicalDevice, logicalDevice, instance};

		const vkx::CommandSubmitter commandSubmitter{physicalDevice, logicalDevice, surface};

		const vkx::SwapchainInfo swapchainInfo{physicalDevice, surface};

		const auto clearRenderPass = vkx::createRenderPassUnique(logicalDevice, physicalDevice, swapchainInfo.chooseSurfaceFormat().format, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear);
		const auto loadRenderPass = vkx::createRenderPassUnique(logicalDevice, physicalDevice, swapchainInfo.chooseSurfaceFormat().format, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::AttachmentLoadOp::eLoad);

		vkx::Swapchain swapchain{logicalDevice, physicalDevice, *clearRenderPass, surface, window, allocator};

		const std::vector poolSizes = {vkx::UNIFORM_BUFFER_POOL_SIZE, vkx::SAMPLER_BUFFER_POOL_SIZE, vkx::UNIFORM_BUFFER_POOL_SIZE, vkx::UNIFORM_BUFFER_POOL_SIZE};

		const std::vector highlightPoolSizes = {vkx::UNIFORM_BUFFER_POOL_SIZE};

		const auto descriptorSetLayout = createShaderDescriptorSetLayout(logicalDevice);

		const auto highlightDescriptorSetLayout = createHighlightDescriptorSetLayout(logicalDevice);

		const vkx::Texture texture{"a.jpg", logicalDevice, maxSamplerAnisotropy, allocator, commandSubmitter};

		const vkx::GraphicsPipelineInformation graphicsPipelineInformation{
		    "shader.vert.spv",
		    "shader.frag.spv",
		    createShaderBindings(),
		    vkx::Vertex::getBindingDescription(),
		    vkx::Vertex::getAttributeDescriptions(),
		    {sizeof(vkx::MVP), sizeof(vkx::DirectionalLight), sizeof(vkx::Material)},
		    {&texture}};

		const vkx::GraphicsPipeline graphicsPipeline{logicalDevice, *clearRenderPass, allocator, graphicsPipelineInformation};

		const vkx::GraphicsPipelineInformation highlightGraphicsPipelineInformation{
		    "highlight.vert.spv",
		    "highlight.frag.spv",
		    createHighlightShaderBindings(),
		    getBindingDescription(),
		    getAttributeDescriptions(),
		    {sizeof(vkx::MVP)},
		    {}};

		const vkx::GraphicsPipeline highlightGraphicsPipeline{logicalDevice, *clearRenderPass, allocator, highlightGraphicsPipelineInformation};

		constexpr std::uint32_t chunkDrawCommandAmount = 1;
		constexpr std::uint32_t highlightDrawCommandAmount = 1;

		constexpr std::uint32_t drawCommandAmount = chunkDrawCommandAmount + highlightDrawCommandAmount;
		constexpr std::uint32_t secondaryDrawCommandAmount = 1;
		const auto drawCommands = commandSubmitter.allocateDrawCommands(drawCommandAmount);
		const auto secondaryDrawCommands = commandSubmitter.allocateSecondaryDrawCommands(secondaryDrawCommandAmount);

		const auto syncObjects = vkx::createSyncObjects(logicalDevice);

		vkx::VoxelChunk<16> chunk{{0, 0, 0}};
		for (int j = 0; j < 10; j++) {
			for (int k = 0; k < 4; k++) {
				for (int i = 0; i < 14; i++) {
					chunk.set(j, k, i, vkx::Voxel::Air);
				}
			}
		}

		chunk.greedy();

		vkx::Mesh mesh{chunk.vertices, chunk.indices, allocator};
		mesh.indexCount = std::distance(chunk.indices.begin(), chunk.indexIter);

		const vkx::Mesh highlightMesh{vkx::CUBE_VERTICES, vkx::CUBE_INDICES, allocator};

		auto mvpBuffers = graphicsPipeline.getUniformByIndex(0);
		auto lightBuffers = graphicsPipeline.getUniformByIndex(1);
		auto materialBuffers = graphicsPipeline.getUniformByIndex(2);

		auto highlightMVPBuffers = highlightGraphicsPipeline.getUniformByIndex(0);

		auto proj = glm::perspective(70.0f, 640.0f / 480.0f, 0.1f, 100.0f);

		auto highlightModel = glm::mat4(1.0f);

		std::uint32_t currentFrame = 0;
		SDL_Event event{};
		bool isRunning = true;
		bool framebufferResized = false;
		SDL_ShowWindow(window);
		while (isRunning) {
			camera.position += camera.direction * 0.001f;

			auto& mvpBuffer = mvpBuffers[currentFrame];
			auto mvp = vkx::MVP{glm::mat4(1.0f), camera.viewMatrix(), proj};

			auto& lightBuffer = lightBuffers[currentFrame];
			auto light = vkx::DirectionalLight{
			    glm::vec3(1.0f, 3.0f, 1.0f),
			    camera.position,
			    glm::vec4(1.0f, 1.0f, 1.0f, 0.2f),
			    glm::vec3(1.0f, 1.0f, 1.0f),
			    glm::vec3(1.0f, 1.0f, 1.0f),
			    1.0f,
			    0.09f,
			    0.032f};

			auto& materialBuffer = materialBuffers[currentFrame];
			auto material = vkx::Material{glm::vec3(0.2f), 100.0f};

			auto& highlightMVPBuffer = highlightMVPBuffers[currentFrame];
			auto highlightMVP = vkx::MVP{highlightModel, mvp.view, mvp.proj};

			mvpBuffer.mapMemory(mvp);
			lightBuffer.mapMemory(light);
			materialBuffer.mapMemory(material);
			highlightMVPBuffer.mapMemory(highlightMVP);

			const auto& syncObject = syncObjects[currentFrame];
			syncObject.waitForFence();
			auto [result, imageIndex] = swapchain.acquireNextImage(logicalDevice, syncObject);

			if (result == vk::Result::eErrorOutOfDateKHR) {
				int newWidth, newHeight;
				SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
				while (newWidth == 0 || newHeight == 0) {
					SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
					SDL_WaitEvent(nullptr);
				}

				vkDeviceWaitIdle(logicalDevice);

				swapchain = vkx::Swapchain{logicalDevice, physicalDevice, *clearRenderPass, surface, window, allocator};
				continue;
			} else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
				throw std::runtime_error("Failed to acquire next image.");
			}

			syncObject.resetFence();

			const vkx::DrawInfo chunkDrawInfo = {
			    imageIndex,
			    currentFrame,
			    &swapchain,
			    &graphicsPipeline,
			    *clearRenderPass,
			    {mesh.vertex->object},
			    {mesh.index->object},
			    {static_cast<std::uint32_t>(mesh.indexCount)}};

			const vkx::DrawInfo highlightDrawInfo = {
			    imageIndex,
			    currentFrame,
			    &swapchain,
			    &highlightGraphicsPipeline,
			    *loadRenderPass,
			    {highlightMesh.vertex->object},
			    {highlightMesh.index->object},
			    {static_cast<std::uint32_t>(highlightMesh.indexCount)}};

			const vk::CommandBuffer* begin = &drawCommands[currentFrame * drawCommandAmount];

			const vk::CommandBuffer* chunkBegin = begin;

			const vk::CommandBuffer* highlightBegin = chunkBegin + chunkDrawCommandAmount;

			const vk::CommandBuffer* secondaryBegin = &secondaryDrawCommands[currentFrame * secondaryDrawCommandAmount];

			commandSubmitter.recordSecondaryDrawCommands(chunkBegin, chunkDrawCommandAmount, secondaryBegin, secondaryDrawCommandAmount, chunkDrawInfo);

			commandSubmitter.recordPrimaryDrawCommands(highlightBegin, highlightDrawCommandAmount, highlightDrawInfo);

			commandSubmitter.submitDrawCommands(begin, drawCommandAmount, syncObject);

			result = commandSubmitter.presentToSwapchain(swapchain, imageIndex, syncObject);

			if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
				framebufferResized = false;

				int newWidth, newHeight;
				SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
				while (newWidth == 0 || newHeight == 0) {
					SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
					SDL_WaitEvent(nullptr);
				}

				vkDeviceWaitIdle(logicalDevice);

				swapchain = vkx::Swapchain{logicalDevice, physicalDevice, *clearRenderPass, surface, window, allocator};
			} else if (result != vk::Result::eSuccess) {
				throw std::runtime_error("Failed to present.");
			}

			currentFrame = (currentFrame + 1) % vkx::MAX_FRAMES_IN_FLIGHT;

			vkx::RaycastResult raycastResult{};
			auto a = [&chunk](const auto& b) {
				return chunk.at(b) != vkx::Voxel::Air;
			};
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_QUIT:
					isRunning = false;
					break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
						framebufferResized = true;
						int width = event.window.data1;
						int height = event.window.data2;
						proj = glm::perspective(70.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
					}
					break;
				case SDL_MOUSEMOTION: {
					camera.updateMouse({-event.motion.xrel, -event.motion.yrel});
					const auto origin = glm::vec3(chunk.normalizedPosition) - camera.position;

					raycastResult = vkx::raycast(origin, camera.front, 4.0f, a);
					if (raycastResult.success) {
						highlightModel = glm::translate(glm::mat4(1.0f), glm::vec3(chunk.normalizedPosition - raycastResult.hitPos) + glm::vec3(-1.0f));
					}
				} break;
				case SDL_KEYDOWN: {
					camera.updateKey(event.key.keysym.sym);
					const auto origin = glm::vec3(chunk.normalizedPosition) - camera.position;
					const vkx::AABB box{glm::vec3{0}, glm::vec3{1}};
					const auto collision = vkx::handleCollision(box, origin, glm::vec3{0, 1, 0}, 1.0f, a);
					if (collision.success) {
						SDL_Log("Colliding");
					}
				} break;
				case SDL_KEYUP:
					camera.direction = glm::vec3(0);
					break;
				case SDL_MOUSEBUTTONDOWN: {
					const auto origin = glm::vec3(chunk.normalizedPosition) - camera.position;
					raycastResult = vkx::raycast(origin, camera.front, 4.0f, a);
					if (raycastResult.success) {
						chunk.set(raycastResult.hitPos, vkx::Voxel::Air);
						chunk.greedy();
						mesh.vertex->mapMemory(chunk.vertices);
						mesh.index->mapMemory(chunk.indices);
						mesh.indexCount = std::distance(chunk.indices.begin(), chunk.indexIter);
					}
				} break;
				default:
					break;
				}
			}
		}
		vkDeviceWaitIdle(logicalDevice);
	}

	vkDestroyDevice(logicalDevice, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
