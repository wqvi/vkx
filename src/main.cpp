#include "vkx/renderer/core/renderer_base.hpp"
#include "vkx/renderer/core/renderer_types.hpp"
#include "vkx/renderer/core/vertex.hpp"
#include "vkx/renderer/model.hpp"
#include "vkx/renderer/uniform_buffer.hpp"
#include "vkx/voxels/voxels.hpp"
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
		vkx::RendererBase renderer(window);

		vkx::VoxelChunk chunk({0, 0, 0}, 16);
		chunk.greedy();

		const vkx::Model model(renderer.allocateMesh(chunk.vertices, chunk.indices), renderer.allocateTexture("a.jpg"), {glm::vec3(0.2f), 100.0f});

		std::vector<vkx::UniformBuffer<vkx::MVP>> mvpBuffers = renderer.createBuffers(vkx::MVP{});
		std::vector<vkx::UniformBuffer<vkx::DirectionalLight>> lightBuffers = renderer.createBuffers(vkx::DirectionalLight{});
		std::vector<vkx::UniformBuffer<vkx::Material>> materialBuffers = renderer.createBuffers(vkx::Material{});
		renderer.createDescriptorSets(mvpBuffers, lightBuffers, materialBuffers, model.texture);

		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		glm::mat4 proj = glm::perspective(70.0f, 640.0f / 480.0f, 0.1f, 100.0f);

		SDL_Event event{};
		bool isRunning = true;
		SDL_ShowWindow(window);
		while (isRunning) {
			auto currentFrame = renderer.getCurrentFrameIndex();

			auto& mvpBuffer = mvpBuffers[currentFrame];
			mvpBuffer->model = model.getModelMatrix();
			mvpBuffer->view = view;
			mvpBuffer->proj = proj;

			auto& lightBuffer = lightBuffers[currentFrame];
			lightBuffer->position = glm::vec3(1.0f, 3.0f, 1.0f);
			lightBuffer->eyePosition = glm::vec3(0.0f, 0.0f, -1.0f);
			lightBuffer->ambientColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.2f);
			lightBuffer->diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
			lightBuffer->specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
			lightBuffer->constant = 1.0f;
			lightBuffer->linear = 0.09f;
			lightBuffer->quadratic = 0.032f;

			auto& materialBuffer = materialBuffers[currentFrame];
			materialBuffer->specularColor = model.material.specularColor;
			materialBuffer->shininess = model.material.shininess;

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
						// proj = glm::ortho(0.0f, static_cast<float>(event.window.data1), static_cast<float>(event.window.data2), 0.0f, 0.1f, 100.0f);
						proj = glm::perspective(70.0f, static_cast<float>(event.window.data1) / static_cast<float>(event.window.data2), 0.1f, 100.0f);
					}
					break;
				}
			}
		}

		renderer.waitIdle();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
