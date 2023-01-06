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

template <class T>
constexpr auto posMod(T x, T y) {
	float value = std::fmod(x, y);
	if (((value < 0) && (y > 0)) || ((value > 0) && (y < 0))) {
		value += y;
	}
	value += 0.0f;
	return value;
};


int main(int argc, char** argv) {
	const vkx::Window window{"vkx", 640, 480};

	vkx::Camera2D camera{glm::vec2{0, 0}, glm::vec2{0, 0}, glm::vec2{0.5f, 0.5f}};

	const vkx::VulkanInstance vulkanInstance{window};

	const auto vulkanDevice = vulkanInstance.createDevice();

	const auto allocator = vulkanDevice.createAllocator();

	const auto swapchainInfo = vulkanDevice.getSwapchainInfo(window);

	const auto clearRenderPass = vulkanDevice.createRenderPass(swapchainInfo.surfaceFormat, vk::AttachmentLoadOp::eClear, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

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

	constexpr std::uint32_t chunkDrawCommandAmount = 4;

	constexpr std::uint32_t drawCommandAmount = 1;
	constexpr std::uint32_t secondaryDrawCommandAmount = chunkDrawCommandAmount;
	const auto drawCommands = commandSubmitter.allocateDrawCommands(drawCommandAmount);
	const auto secondaryDrawCommands = commandSubmitter.allocateSecondaryDrawCommands(secondaryDrawCommandAmount);

	const auto syncObjects = vulkanDevice.createSyncObjects();

	std::vector<vkx::VoxelChunk2D> chunks{};
	chunks.reserve(4);
	chunks.emplace_back(glm::vec2{0.0f, 0.0f});
	chunks.emplace_back(glm::vec2{1.0f, 0.0f});
	chunks.emplace_back(glm::vec2{1.0f, 1.0f});
	chunks.emplace_back(glm::vec2{0.0f, 1.0f});

	chunks[0].generateTerrain();
	chunks[1].generateTerrain();
	chunks[2].generateTerrain();
	chunks[3].generateTerrain();

	std::vector<vkx::Mesh> meshes{};
	meshes.reserve(4);
	meshes.emplace_back(vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 4, vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 6, allocator);
	meshes.emplace_back(vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 4, vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 6, allocator);
	meshes.emplace_back(vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 4, vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 6, allocator);
	meshes.emplace_back(vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 4, vkx::CHUNK_SIZE * vkx::CHUNK_SIZE * 6, allocator);

	chunks[0].generateMesh(meshes[0]);
	chunks[1].generateMesh(meshes[1]);
	chunks[2].generateMesh(meshes[2]);
	chunks[3].generateMesh(meshes[3]);

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

		if (std::abs(right - left)) {
			direction.x = 0.0f;
		}

		if (std::abs(up - down)) {
			direction.y = 0.0f;
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
			default:
				break;
			}
		}

		camera.globalPosition += direction;

		// Update game
		for (auto i = 0; i < 4; i++) {
			auto& chunk = chunks[i];
			auto& mesh = meshes[i];

			const auto chunkX = glm::floor(chunk.getGlobalPosition().x / static_cast<float>(vkx::CHUNK_SIZE));
			const auto chunkY = glm::floor(chunk.getGlobalPosition().y / static_cast<float>(vkx::CHUNK_SIZE));
		
			const auto playerX = glm::floor(camera.globalPosition.x / static_cast<float>(vkx::CHUNK_SIZE));
			const auto playerY = glm::floor(camera.globalPosition.y / static_cast<float>(vkx::CHUNK_SIZE));
		
			const auto newX = posMod(chunkX - playerX + vkx::CHUNK_HALF_RADIUS, vkx::CHUNK_RADIUS) + playerX - vkx::CHUNK_HALF_RADIUS;
			const auto newY = posMod(chunkY - playerY + vkx::CHUNK_HALF_RADIUS, vkx::CHUNK_RADIUS) + playerY - vkx::CHUNK_HALF_RADIUS;

			if (newX != chunkX || newY != chunkY) {
				chunk.setGlobalPosition({newX, newY});
				chunk.generateTerrain();
				chunk.generateMesh(mesh);
			}
		}

		// Render
		auto& mvpBuffer = mvpBuffers[currentFrame];
		auto mvp = vkx::MVP{glm::mat4(1.0f), camera.viewMatrix(), projection};

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

		mvpBuffer.mapMemory(mvp);
		lightBuffer.mapMemory(light);
		materialBuffer.mapMemory(material);

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

		const vkx::DrawInfo chunkDrawInfo = {
		    imageIndex,
		    currentFrame,
		    &swapchain,
		    &graphicsPipeline,
		    static_cast<vk::RenderPass>(clearRenderPass),
		    meshes};

		const auto* begin = &drawCommands[currentFrame * drawCommandAmount];
		const auto* secondaryBegin = &secondaryDrawCommands[currentFrame * secondaryDrawCommandAmount];

		commandSubmitter.recordSecondaryDrawCommands(begin, drawCommandAmount, secondaryBegin, chunkDrawCommandAmount, chunkDrawInfo);

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
