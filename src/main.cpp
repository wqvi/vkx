#include <vkx/vkx.hpp>

auto createShaderDescriptorSetLayout(VkDevice device) {
	constexpr VkDescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    1,
	    VK_SHADER_STAGE_VERTEX_BIT,
	    nullptr};

	constexpr VkDescriptorSetLayoutBinding samplerLayoutBinding{
	    1,
	    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	    1,
	    VK_SHADER_STAGE_FRAGMENT_BIT,
	    nullptr};

	constexpr VkDescriptorSetLayoutBinding lightLayoutBinding{
	    2,
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    1,
	    VK_SHADER_STAGE_FRAGMENT_BIT,
	    nullptr};

	constexpr VkDescriptorSetLayoutBinding materialLayoutBinding{
	    3,
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    1,
	    VK_SHADER_STAGE_FRAGMENT_BIT,
	    nullptr};

	constexpr VkDescriptorSetLayoutBinding bindings[4] = {uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding, materialLayoutBinding};

	const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
	    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	    nullptr,
	    0,
	    4,
	    bindings};

	VkDescriptorSetLayout descriptorSetLayout = nullptr;
	if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout.");
	}

	return descriptorSetLayout;
}

auto createShaderBindings() {
	constexpr VkDescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    1,
	    VK_SHADER_STAGE_VERTEX_BIT,
	    nullptr};

	constexpr VkDescriptorSetLayoutBinding samplerLayoutBinding{
	    1,
	    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	    1,
	    VK_SHADER_STAGE_FRAGMENT_BIT,
	    nullptr};

	constexpr VkDescriptorSetLayoutBinding lightLayoutBinding{
	    2,
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    1,
	    VK_SHADER_STAGE_FRAGMENT_BIT,
	    nullptr};

	constexpr VkDescriptorSetLayoutBinding materialLayoutBinding{
	    3,
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    1,
	    VK_SHADER_STAGE_FRAGMENT_BIT,
	    nullptr};

	return std::vector{uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding, materialLayoutBinding};
}

auto createHighlightDescriptorSetLayout(VkDevice device) {
	constexpr VkDescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    1,
	    VK_SHADER_STAGE_VERTEX_BIT,
	    nullptr};

	const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
	    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	    nullptr,
	    0,
	    1,
	    &uboLayoutBinding};

	VkDescriptorSetLayout descriptorSetLayout = nullptr;
	if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout.");
	}

	return descriptorSetLayout;
}

auto createHighlightShaderBindings() noexcept {
	constexpr VkDescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    1,
	    VK_SHADER_STAGE_VERTEX_BIT,
	    nullptr};

	return std::vector{uboLayoutBinding};
}

auto getBindingDescription() noexcept {
	std::vector<VkVertexInputBindingDescription> bindingDescriptions{};

	bindingDescriptions.push_back({0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX});

	return bindingDescriptions;
}

auto getAttributeDescriptions() noexcept {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

	attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0});

	return attributeDescriptions;
}

int main(int argc, char** argv) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
		return EXIT_FAILURE;
	}

	if (SDL_SetRelativeMouseMode(SDL_TRUE)) {
		return EXIT_FAILURE;
	}

	SDL_Window* window = SDL_CreateWindow("vkx", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
	if (window == nullptr) {
		SDL_Log("%s", SDL_GetError());
		return EXIT_FAILURE;
	}

	vkx::Camera camera{0.0f, 0.0f, 0.0f};

	const auto instance = vkx::createInstance(window);
	const auto surface = vkx::createSurface(window, instance);
	const auto physicalDevice = vkx::getBestPhysicalDevice(instance, surface);
	const auto properties = vkx::getObject<VkPhysicalDeviceProperties>(vkGetPhysicalDeviceProperties, physicalDevice);
	const auto logicalDevice = vkx::createDevice(instance, surface, physicalDevice);
	const vkx::SwapchainInfo swapchainInfo{physicalDevice, surface};
	const auto clearRenderPass = vkx::createRenderPass(physicalDevice, logicalDevice, swapchainInfo.chooseSurfaceFormat().format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
	const auto loadRenderPass = vkx::createRenderPass(physicalDevice, logicalDevice, swapchainInfo.chooseSurfaceFormat().format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_LOAD);
	const vkx::QueueConfig queueConfig{physicalDevice, surface};
	const auto allocator = vkx::createAllocator(physicalDevice, logicalDevice, instance);
	const vkx::CommandSubmitter commandSubmitter{physicalDevice, logicalDevice, surface};
	vkx::Swapchain swapchain{physicalDevice, logicalDevice, clearRenderPass, surface, allocator, window};

	const std::vector poolSizes = {vkx::UNIFORM_BUFFER_POOL_SIZE, vkx::SAMPLER_BUFFER_POOL_SIZE, vkx::UNIFORM_BUFFER_POOL_SIZE, vkx::UNIFORM_BUFFER_POOL_SIZE};
	const std::vector highlightPoolSizes = {vkx::UNIFORM_BUFFER_POOL_SIZE};
	const auto descriptorSetLayout = createShaderDescriptorSetLayout(logicalDevice);
	const auto highlightDescriptorSetLayout = createHighlightDescriptorSetLayout(logicalDevice);

	const vkx::Texture texture{"a.jpg", logicalDevice, properties.limits.maxSamplerAnisotropy, allocator, commandSubmitter};

	const vkx::GraphicsPipelineInformation graphicsPipelineInformation{
	    "shader.vert.spv",
	    "shader.frag.spv",
	    createShaderBindings(),
	    vkx::Vertex::getBindingDescription(),
	    vkx::Vertex::getAttributeDescriptions(),
	    {sizeof(vkx::MVP), sizeof(vkx::DirectionalLight), sizeof(vkx::Material)},
	    {&texture}};

	const vkx::GraphicsPipeline graphicsPipeline{logicalDevice, clearRenderPass, allocator, graphicsPipelineInformation};

	const vkx::GraphicsPipelineInformation highlightGraphicsPipelineInformation{
	    "highlight.vert.spv",
	    "highlight.frag.spv",
	    createHighlightShaderBindings(),
	    getBindingDescription(),
	    getAttributeDescriptions(),
	    {sizeof(vkx::MVP)},
	    {}};

	const vkx::GraphicsPipeline highlightGraphicsPipeline{logicalDevice, clearRenderPass, allocator, highlightGraphicsPipelineInformation};

	constexpr std::uint32_t chunkDrawCommandAmount = 1;
	constexpr std::uint32_t highlightDrawCommandAmount = 1;

	constexpr std::uint32_t drawCommandAmount = chunkDrawCommandAmount + highlightDrawCommandAmount;
	constexpr std::uint32_t secondaryDrawCommandAmount = 1;
	const auto drawCommands = commandSubmitter.allocateDrawCommands(drawCommandAmount);
	const auto secondaryDrawCommands = commandSubmitter.allocateSecondaryDrawCommands(secondaryDrawCommandAmount);

	const auto syncObjects = vkx::createSyncObjects(logicalDevice);

	vkx::VoxelChunk chunk{8, {0, 0, 0}};
	for (int j = 0; j < 10; j++) {
		for (int k = 0; k < 4; k++) {
			for (int i = 0; i < 14; i++) {
				chunk.set(j, k, i, vkx::Voxel::Air);
			}
		}
	}

	chunk.greedy();

	vkx::Mesh mesh{chunk.vertices, chunk.indices, allocator};
	mesh.indexCount = static_cast<std::uint32_t>(std::distance(chunk.indices.begin(), chunk.indexIter));

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
		camera.position += camera.direction * 0.01f;

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
		std::uint32_t imageIndex = 0;
		auto result = swapchain.acquireNextImage(logicalDevice, syncObject, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			int newWidth, newHeight;
			SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
			while (newWidth == 0 || newHeight == 0) {
				SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
				SDL_WaitEvent(nullptr);
			}

			vkDeviceWaitIdle(logicalDevice);

			swapchain.destroy();

			swapchain = vkx::Swapchain{physicalDevice, logicalDevice, clearRenderPass, surface, allocator, window};
			continue;
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire next image.");
		}

		syncObject.resetFence();

		const vkx::DrawInfo chunkDrawInfo = {
		    imageIndex,
		    currentFrame,
		    &swapchain,
		    &graphicsPipeline,
		    clearRenderPass,
		    {mesh.vertexBuffer},
		    {mesh.indexBuffer},
		    {static_cast<std::uint32_t>(mesh.indexCount)}};

		const vkx::DrawInfo highlightDrawInfo = {
		    imageIndex,
		    currentFrame,
		    &swapchain,
		    &highlightGraphicsPipeline,
		    loadRenderPass,
		    {highlightMesh.vertexBuffer},
		    {highlightMesh.indexBuffer},
		    {static_cast<std::uint32_t>(highlightMesh.indexCount)}};

		const auto* begin = &drawCommands[currentFrame * drawCommandAmount];

		const auto* chunkBegin = begin;

		const auto* highlightBegin = chunkBegin + chunkDrawCommandAmount;

		const auto* secondaryBegin = &secondaryDrawCommands[currentFrame * secondaryDrawCommandAmount];

		commandSubmitter.recordSecondaryDrawCommands(chunkBegin, chunkDrawCommandAmount, secondaryBegin, secondaryDrawCommandAmount, chunkDrawInfo);

		commandSubmitter.recordPrimaryDrawCommands(highlightBegin, highlightDrawCommandAmount, highlightDrawInfo);

		commandSubmitter.submitDrawCommands(begin, drawCommandAmount, syncObject);

		result = commandSubmitter.presentToSwapchain(swapchain, imageIndex, syncObject);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;

			int newWidth, newHeight;
			SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
			while (newWidth == 0 || newHeight == 0) {
				SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
				SDL_WaitEvent(nullptr);
			}

			vkDeviceWaitIdle(logicalDevice);

			swapchain.destroy();

			swapchain = vkx::Swapchain{physicalDevice, logicalDevice, clearRenderPass, surface, allocator, window};
		} else if (result != VK_SUCCESS) {
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

				if (event.key.keysym.sym == SDLK_ESCAPE) {
					isRunning = false;
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
					std::memcpy(mesh.vertexAllocationInfo.pMappedData, chunk.vertices.data(), mesh.vertexAllocationInfo.size);
					std::memcpy(mesh.indexAllocationInfo.pMappedData, chunk.indices.data(), mesh.indexAllocationInfo.size);
					mesh.indexCount = static_cast<std::uint32_t>(std::distance(chunk.indices.begin(), chunk.indexIter));
				}
			} break;
			default:
				break;
			}
		}
	}
	vkDeviceWaitIdle(logicalDevice);

	for (const auto& syncObject : syncObjects) {
		syncObject.destroy();
	}

	texture.destroy();
	mesh.destroy(allocator);
	highlightMesh.destroy(allocator);
	swapchain.destroy();
	commandSubmitter.destroy();
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
	graphicsPipeline.destroy();
	vkDestroyDescriptorSetLayout(logicalDevice, highlightDescriptorSetLayout, nullptr);
	highlightGraphicsPipeline.destroy();

	vmaDestroyAllocator(allocator);
	vkDestroyRenderPass(logicalDevice, clearRenderPass, nullptr);
	vkDestroyRenderPass(logicalDevice, loadRenderPass, nullptr);
	vkDestroyDevice(logicalDevice, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
