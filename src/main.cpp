#include <vkx/vkx.hpp>

auto createShaderBindings() {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex};

	constexpr vk::DescriptorSetLayoutBinding samplerLayoutBinding{
	    1,
	    vk::DescriptorType::eCombinedImageSampler,
	    1,
	    vk::ShaderStageFlagBits::eFragment};

	constexpr vk::DescriptorSetLayoutBinding lightLayoutBinding{
	    2,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment};

	constexpr vk::DescriptorSetLayoutBinding materialLayoutBinding{
	    3,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment};

	return std::vector{uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding, materialLayoutBinding};
}

auto createHighlightShaderBindings() {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex};

	return std::vector{uboLayoutBinding};
}

int main(int argc, char** argv) {
	const vkx::Window window{"vkx", 640, 480};

	vkx::Camera2D camera{glm::vec2{0, 0}, glm::vec2{0, 0}, glm::vec2{0.5f, 0.5f}};

	const vkx::VulkanInstance vulkanInstance{window};

	const auto vulkanDevice = vulkanInstance.createDevice();

	const auto allocator = vulkanDevice.createAllocator();

	const auto swapchainInfo = vulkanDevice.getSwapchainInfo(window);

	const auto loadRenderPass = vulkanDevice.createRenderPass(swapchainInfo.surfaceFormat, vk::AttachmentLoadOp::eLoad, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR);

	const auto clearRenderPass = vulkanDevice.createRenderPass(swapchainInfo.surfaceFormat, vk::AttachmentLoadOp::eClear, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

	auto swapchain = vulkanDevice.createSwapchain(allocator, clearRenderPass, window);

	const auto commandSubmitter = vulkanDevice.createCommandSubmitter();

	const vkx::Texture texture{"a.jpg", vulkanDevice, allocator, commandSubmitter};

	const vkx::GraphicsPipelineInformation graphicsPipelineInformation{
	    "shader2D.vert.spv",
	    "shader2D.frag.spv",
	    createShaderBindings(),
	    vkx::Vertex::getBindingDescription(),
	    vkx::Vertex::getAttributeDescriptions(),
	    {sizeof(vkx::MVP), sizeof(vkx::DirectionalLight), sizeof(vkx::Material)},
	    {&texture}};
	const auto graphicsPipeline = vulkanDevice.createGraphicsPipeline(clearRenderPass, allocator, graphicsPipelineInformation);

	const vkx::GraphicsPipelineInformation highlightGraphicsPipelineInformation{
	    "highlight.vert.spv",
	    "highlight.frag.spv",
	    createHighlightShaderBindings(),
	    vkx::Vertex::getBindingDescription(),
	    vkx::Vertex::getAttributeDescriptions(),
	    {sizeof(vkx::MVP)},
	    {}};
	const auto highlightGraphicsPipeline = vulkanDevice.createGraphicsPipeline(loadRenderPass, allocator, highlightGraphicsPipelineInformation);

	constexpr std::uint32_t chunkDrawCommandAmount = static_cast<std::uint32_t>(vkx::CHUNK_RADIUS * vkx::CHUNK_RADIUS);

	constexpr std::uint32_t drawCommandAmount = 2;
	constexpr std::uint32_t secondaryDrawCommandAmount = chunkDrawCommandAmount;
	const auto drawCommands = commandSubmitter.allocateDrawCommands(drawCommandAmount);
	const auto secondaryDrawCommands = commandSubmitter.allocateDrawCommands(secondaryDrawCommandAmount, vk::CommandBufferLevel::eSecondary);

	const auto syncObjects = vulkanDevice.createSyncObjects();

	std::vector<vkx::VoxelChunk2D> chunks{};
	std::vector<vkx::Mesh> meshes{};
	chunks.reserve(static_cast<std::size_t>(vkx::CHUNK_RADIUS * vkx::CHUNK_RADIUS));
	meshes.reserve(static_cast<std::size_t>(vkx::CHUNK_RADIUS * vkx::CHUNK_RADIUS));
	for (auto y = 0; y < vkx::CHUNK_RADIUS; y++) {
		for (auto x = 0; x < vkx::CHUNK_RADIUS; x++) {
			auto& currentChunk = chunks.emplace_back(glm::vec2{x, y});
			currentChunk.generateTerrain();
			auto& currentMesh = meshes.emplace_back(vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 4, vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 6, allocator);
			currentChunk.generateMesh(currentMesh);
		}
	}

	std::vector<vkx::Vertex> vertices{vkx::Vertex{{0, 0}}, vkx::Vertex{{16, 0}},
					  vkx::Vertex{{16, 16}}, vkx::Vertex{{0, 16}}};
	std::vector<std::uint32_t> indices{0, 1, 2, 2, 3, 0};
	std::vector<vkx::Mesh> highlightMeshes{};
	highlightMeshes.reserve(1);
	highlightMeshes.emplace_back(std::move(vertices), std::move(indices), 6, allocator);

	auto& mvpBuffers = graphicsPipeline.getUniformByIndex(0);
	auto& lightBuffers = graphicsPipeline.getUniformByIndex(1);
	auto& materialBuffers = graphicsPipeline.getUniformByIndex(2);

	auto& highlightMVPBuffers = highlightGraphicsPipeline.getUniformByIndex(0);

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

	glm::vec2 direction{0};

	const auto sdlKeyPressedEvent = [&isRunning, &direction](const SDL_KeyboardEvent& key) {
		if (key.keysym.sym == SDLK_ESCAPE) {
			isRunning = false;
		}

		const auto left = key.keysym.sym == SDLK_a;
		const auto right = key.keysym.sym == SDLK_d;

		const auto up = key.keysym.sym == SDLK_w;
		const auto down = key.keysym.sym == SDLK_s;

		const auto xDirection = right - left;
		const auto yDirection = up - down;

		if (std::abs(xDirection)) {
			direction.x = static_cast<float>(xDirection);
		}

		if (std::abs(yDirection)) {
			direction.y = static_cast<float>(yDirection);
		}
	};

	const auto sdlKeyReleasedEvent = [&direction](const SDL_KeyboardEvent& key) {
		const auto left = key.keysym.sym == SDLK_a;
		const auto right = key.keysym.sym == SDLK_d;

		const auto up = key.keysym.sym == SDLK_w;
		const auto down = key.keysym.sym == SDLK_s;

		const auto xDirection = right - left;
		const auto yDirection = up - down;

		if (std::abs(xDirection)) {
			direction.x = 0.0f;
		}

		if (std::abs(yDirection)) {
			direction.y = 0.0f;
		}
	};

	glm::mat4 highlightMatrix {1.0f};

	const auto sdlMouseMotionEvent = [&chunks, &camera, &highlightMatrix](const SDL_MouseMotionEvent& motion) {
		for (const auto& chunk : chunks) {
			// Raycast from player position to where the mouse is.
			// Raycasting api must be made 2D
			// Add another pipeline for highlighting stuff
			const glm::vec2 mousePosition{motion.x, motion.y};

			const auto result = vkx::raycast2D(camera.globalPosition,
							   mousePosition,
							   4,
							   [&chunk, &mousePosition](auto pos) {
								   const auto index = static_cast<std::size_t>(pos.x + pos.y * static_cast<float>(vkx::CHUNK_SIZE));
								   if (index < vkx::CHUNK_SIZE * vkx::CHUNK_SIZE) {
									   const auto currentVoxel = chunk.voxels[index];
									   return currentVoxel != vkx::Voxel::Air;
								   }
								   return false;
							   });

			highlightMatrix = glm::mat4(glm::translate(glm::mat3(1.0f), result.hitPosition * 16.0f));
		}
	};

	window.show();
	while (isRunning) {
		// Poll events
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
			case SDL_KEYUP:
				sdlKeyReleasedEvent(event.key);
				break;
			case SDL_MOUSEMOTION:
				sdlMouseMotionEvent(event.motion);
				break;
			default:
				break;
			}
		}

		camera.globalPosition += direction;

		const auto playerGlobalPosition = camera.globalPosition;
		const auto playerX = glm::floor(playerGlobalPosition.x / vkx::CHUNK_SIZE);
		const auto playerY = glm::floor(playerGlobalPosition.y / vkx::CHUNK_SIZE);

		// Update game
		for (auto i = 0; i < vkx::CHUNK_RADIUS * vkx::CHUNK_RADIUS; i++) {
			auto& chunk = chunks[i];
			auto& mesh = meshes[i];

			const auto& chunkGlobalPosition = chunk.globalPosition;

			const auto chunkX = glm::floor(chunkGlobalPosition.x / vkx::CHUNK_SIZE);
			const auto chunkY = glm::floor(chunkGlobalPosition.y / vkx::CHUNK_SIZE);

			const auto newX = glm::floor(vkx::posMod(chunkX - playerX + vkx::CHUNK_HALF_RADIUS, vkx::CHUNK_RADIUS) + playerX - vkx::CHUNK_HALF_RADIUS);
			const auto newY = glm::floor(vkx::posMod(chunkY - playerY + vkx::CHUNK_HALF_RADIUS, vkx::CHUNK_RADIUS) + playerY - vkx::CHUNK_HALF_RADIUS);

			if (newX != chunkX || newY != chunkY) {
				chunk.globalPosition = {newX * vkx::CHUNK_SIZE, newY * vkx::CHUNK_SIZE}; // Check the journal entry about this!
				chunk.generateTerrain();
				chunk.generateMesh(mesh);
			}
		}

		// Render
		const auto [windowWidth, windowHeight] = window.getDimensions();
		const glm::vec2 windowCenter{windowWidth / 2, windowHeight / 2};

		auto& mvpBuffer = mvpBuffers[currentFrame];
		auto mvp = vkx::MVP{glm::mat4(glm::translate(glm::mat3(1.0f), windowCenter)), camera.viewMatrix(), projection};

		auto& lightBuffer = lightBuffers[currentFrame];
		auto light = vkx::DirectionalLight{
		    glm::vec3(1.0f, 3.0f, 1.0f),
		    glm::vec3(0),
		    glm::vec4(1.0f, 1.0f, 1.0f, 0.2f),
		    glm::vec3(1.0f, 1.0f, 1.0f),
		    glm::vec3(1.0f, 1.0f, 1.0f),
		    1.0f,
		    0.09f,
		    0.032f};

		auto& materialBuffer = materialBuffers[currentFrame];
		auto material = vkx::Material{glm::vec3(0.2f), 100.0f};

		auto& highlightMVPBuffer = highlightMVPBuffers[currentFrame];
		auto highlightMVP = vkx::MVP{highlightMatrix, camera.viewMatrix(), projection};

		mvpBuffer.mapMemory(mvp);
		lightBuffer.mapMemory(light);
		materialBuffer.mapMemory(material);

		highlightMVPBuffer.mapMemory(highlightMVP);

		const auto& syncObject = syncObjects[currentFrame];
		syncObject.waitForFence();
		auto [result, imageIndex] = swapchain.acquireNextImage(syncObject);

		if (result == vk::Result::eErrorOutOfDateKHR) {
			window.waitForUpdate();

			vulkanDevice.waitIdle();

			swapchain = vulkanDevice.createSwapchain(allocator, clearRenderPass, window);
			continue;
		} else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
			throw std::runtime_error("Failed to acquire next image.");
		}

		syncObject.resetFence();

		const vkx::DrawInfo chunkDrawInfo{
		    imageIndex,
		    currentFrame,
		    &swapchain,
		    &graphicsPipeline,
		    &clearRenderPass,
		    meshes};

		const vkx::DrawInfo highlightDrawInfo{
		    imageIndex,
		    currentFrame,
		    &swapchain,
		    &highlightGraphicsPipeline,
		    &loadRenderPass,
		    highlightMeshes};

		const auto* begin = &drawCommands[currentFrame * drawCommandAmount];
		const auto* secondaryBegin = &secondaryDrawCommands[currentFrame * secondaryDrawCommandAmount];

		const auto* highlightBegin = &drawCommands[currentFrame * drawCommandAmount + 1];

		commandSubmitter.recordSecondaryDrawCommands(begin, 1, secondaryBegin, chunkDrawCommandAmount, chunkDrawInfo);

		commandSubmitter.recordPrimaryDrawCommands(highlightBegin, 1, highlightDrawInfo);

		commandSubmitter.submitDrawCommands(begin, drawCommandAmount, syncObject);

		result = commandSubmitter.presentToSwapchain(swapchain, imageIndex, syncObject);

		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
			framebufferResized = false;

			window.waitForUpdate();

			vulkanDevice.waitIdle();

			swapchain = vulkanDevice.createSwapchain(allocator, clearRenderPass, window);
		} else if (result != vk::Result::eSuccess) {
			throw std::runtime_error("Failed to present.");
		}

		currentFrame = (currentFrame + 1) % vkx::MAX_FRAMES_IN_FLIGHT;
	}

	vulkanDevice.waitIdle();

	return EXIT_SUCCESS;
}
