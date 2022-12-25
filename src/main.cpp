#include <vkx/vkx.hpp>

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

int main(int argc, char** argv) {
	const vkx::Window window{"vkx", 640, 480};

	vkx::Camera camera{0.0f, 0.0f, 0.0f};

	const vkx::VulkanInstance vulkanInstance{window};

	vkx::VulkanDevice vulkanDevice;
	vulkanDevice = vulkanInstance.createDevice();
	
	vkx::VulkanAllocator allocator;
	allocator = vulkanDevice.createAllocator();
	
	const auto swapchainInfo = vulkanDevice.getSwapchainInfo(window);
	
	vkx::VulkanRenderPass clearRenderPass;
	clearRenderPass = vulkanDevice.createRenderPass(swapchainInfo.surfaceFormat, vk::AttachmentLoadOp::eClear, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
	
	vkx::Swapchain swapchain;
	swapchain = vulkanDevice.createSwapchain(allocator, clearRenderPass, window);
	
	const auto commandSubmitter = vulkanDevice.createCommandSubmitter();
	
	const vkx::Texture texture{"a.jpg", static_cast<VkDevice>(vulkanDevice), vulkanDevice.getMaxSamplerAnisotropy(), allocator, commandSubmitter};
	
	const vkx::GraphicsPipelineInformation graphicsPipelineInformation{
	    "shader2D.vert.spv",
	    "shader2D.frag.spv",
	    createShaderBindings(),
	    vkx::Vertex::getBindingDescription(),
	    vkx::Vertex::getAttributeDescriptions(),
	    {sizeof(vkx::MVP), sizeof(vkx::DirectionalLight), sizeof(vkx::Material)},
	    {&texture}};
	const auto graphicsPipeline = vulkanDevice.createGraphicsPipeline(clearRenderPass, allocator, graphicsPipelineInformation);
	
	constexpr std::uint32_t chunkDrawCommandAmount = 1;

	constexpr std::uint32_t drawCommandAmount = chunkDrawCommandAmount;
	const auto drawCommands = commandSubmitter.allocateDrawCommands(drawCommandAmount);

	const auto syncObjects = vkx::createSyncObjects(static_cast<VkDevice>(vulkanDevice));

	vkx::VoxelChunk2D voxelChunk2D{{0.0f, 0.0f}};
	voxelChunk2D.generateTerrain();
	vkx::Mesh mesh{vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 4, vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 6, allocator};
	voxelChunk2D.generateMesh(mesh);

	auto& mvpBuffers = graphicsPipeline.getUniformByIndex(0);
	auto& lightBuffers = graphicsPipeline.getUniformByIndex(1);
	auto& materialBuffers = graphicsPipeline.getUniformByIndex(2);

	SDL_Event event{};
	bool isRunning = true;
	std::uint32_t currentFrame = 0;
	bool framebufferResized = false;

	glm::mat4 view{1.0f};
	auto projection = glm::ortho(0.0f, 640.0f, 480.0f, 0.0f, 0.1f, 100.0f);

	const auto sdlWindowResizedEvent = [&framebufferResized, &projection](std::int32_t width, std::int32_t height) {
		framebufferResized = true;
		projection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 0.1f, 100.0f);
	};

	const auto sdlWindowEvent = [&sdlWindowResizedEvent](SDL_WindowEvent windowEvent) {
		if (windowEvent.event == SDL_WINDOWEVENT_RESIZED) {
			const auto width = windowEvent.data1;
			const auto height = windowEvent.data2;
			sdlWindowResizedEvent(width, height);
		}
	};

	const auto sdlKeyPressedEvent = [&isRunning](const SDL_KeyboardEvent& key) {
		if (key.keysym.sym == SDLK_ESCAPE) {
			isRunning = false;
		}
	};

	window.show();
	while (isRunning) {
		camera.position += camera.direction * 0.01f;

		auto& mvpBuffer = mvpBuffers[currentFrame];
		auto mvp = vkx::MVP{glm::mat4(1.0f), view, projection};

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

		mvpBuffer.mapMemory(mvp);
		lightBuffer.mapMemory(light);
		materialBuffer.mapMemory(material);

		const auto& syncObject = syncObjects[currentFrame];
		syncObject.waitForFence();
		std::uint32_t imageIndex = 0;
		auto result = swapchain.acquireNextImage(static_cast<VkDevice>(vulkanDevice), syncObject, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			window.waitForUpdate();

			vulkanDevice.waitIdle();

			swapchain = vulkanDevice.createSwapchain(allocator, clearRenderPass, window);
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
		    static_cast<VkRenderPass>(clearRenderPass),
		    {static_cast<VkBuffer>(mesh.vertexBuffer)},
		    {static_cast<VkBuffer>(mesh.indexBuffer)},
		    {static_cast<std::uint32_t>(mesh.activeIndexCount)}};

		const auto* begin = &drawCommands[currentFrame * drawCommandAmount];

		const auto* chunkBegin = begin;

		commandSubmitter.recordPrimaryDrawCommands(chunkBegin, chunkDrawCommandAmount, chunkDrawInfo);

		commandSubmitter.submitDrawCommands(begin, drawCommandAmount, syncObject);

		result = static_cast<VkResult>(commandSubmitter.presentToSwapchain(swapchain, imageIndex, syncObject));

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;

			window.waitForUpdate();

			vulkanDevice.waitIdle();

			swapchain = vulkanDevice.createSwapchain(allocator, clearRenderPass, window);
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present.");
		}

		currentFrame = (currentFrame + 1) % vkx::MAX_FRAMES_IN_FLIGHT;

		while (SDL_PollEvent(&event)) {
			const auto eventType = event.type;

			switch (eventType) {
			case SDL_QUIT:
				isRunning = false;
				break;
			case SDL_WINDOWEVENT:
				sdlWindowEvent(event.window);
				break;
			case SDL_KEYDOWN:
				sdlKeyPressedEvent(event.key);
				break;
			default:
				break;
			}
		}
	}

	vulkanDevice.waitIdle();

	for (const auto& syncObject : syncObjects) {
		syncObject.destroy();
	}

	texture.destroy();
	commandSubmitter.destroy();
	graphicsPipeline.destroy();

	return EXIT_SUCCESS;
}
