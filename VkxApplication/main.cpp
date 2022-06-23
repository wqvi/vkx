//
// Created by december on 6/21/22.
//

#include <vkx.hpp>
#include <iostream>
#include <chrono>
#include <SDL2/SDL_vulkan.h>

const std::uint32_t WIDTH = 800;
const std::uint32_t HEIGHT = 600;

class VoxelRenderer : private vkx::RendererBase {
public:
    explicit VoxelRenderer(vkx::Window const &window)
            : vkx::RendererBase(window, vkx::Profile::createDefault()) {}

    void run() {
        initVulkan();
        mainLoop();
    }

    Camera camera{{2.0f, 2.0f, 2.0f}};

    vkx::Texture texture;

    vkx::Buffer vertexBuffer;
    vkx::Buffer indexBuffer;

    std::vector<vkx::UniformBuffer<vkx::MVP>> mvpBuffers;
    std::vector<vkx::UniformBuffer<vkx::DirectionalLight>> lightBuffers;
    std::vector<vkx::UniformBuffer<vkx::Material>> materialBuffers;

    vkx::VoxelChunk chunk{16, 15, 14};

    glm::mat4 projection = glm::mat4(1.0f);

    void initVulkan() {
        chunk.greedy();

        texture = vkx::Texture{"a.jpg", device};

        vertexBuffer = {chunk.vertices, device};
        indexBuffer = {chunk.indices, device};

        projection = glm::perspective(glm::radians(75.0f), static_cast<float>(swapchain.extent.width) /
                                                           static_cast<float>(swapchain.extent.height), 0.1f, 100.0f);
        projection[1][1] *= -1.0f;

        vkx::MVP mvp{};
        mvp.model = glm::mat4(1.0f);
        mvp.view = camera.viewMatrix();
        mvp.proj = projection;

        vkx::DirectionalLight light{};
        light.position = glm::vec3(1.0f, 3.0f, 1.0f);
        light.eyePosition = camera.position;
        light.ambientColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.2f);
        light.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
        light.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
        light.constant = 1.0f;
        light.linear = 0.09f;
        light.quadratic = 0.032f;

        vkx::Material material{};
        material.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
        material.shininess = 100.0f;

        mvpBuffers = createBuffers(mvp);
        lightBuffers = createBuffers(light);
        materialBuffers = createBuffers(material);

        createDescriptorSets(mvpBuffers, lightBuffers, materialBuffers, texture);
    }

    bool running = true;

    void windowEventHandler(const SDL_WindowEvent &event) {
        switch (event.event) {
            case SDL_WINDOWEVENT_RESIZED:
                framebufferResized = true;
                int width;
                int height;
                SDL_Vulkan_GetDrawableSize(window.internalHandle, &width, &height);
                projection = glm::perspective(glm::radians(75.0f),
                                              static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
                projection[1][1] *= -1.0f;
                break;
        }
    }

    void mouseMovedEventHandler(const SDL_MouseMotionEvent &event) {
        camera.updateMouse({event.xrel, -event.yrel});
    }

    void keyPressedEventHandler(const SDL_KeyboardEvent &event) {
        camera.updateKey(event.keysym.sym);
    }

    void keyReleasedEventHandler(const SDL_KeyboardEvent &event) {
        camera.direction = glm::vec3(0.0f);
    }

    void eventHandler(const SDL_Event &event) {
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                return;
            case SDL_WINDOWEVENT:
                windowEventHandler(event.window);
                break;
            case SDL_KEYDOWN:
                keyPressedEventHandler(event.key);
                break;
            case SDL_KEYUP:
                keyReleasedEventHandler(event.key);
                break;
            case SDL_MOUSEMOTION:
                mouseMovedEventHandler(event.motion);
                break;
            default:
                break;
        }
    }

    void mainLoop() {
        window.show();
        SDL_Event event;
        auto lastTime = std::chrono::high_resolution_clock::now();
        while (running) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto deltaTime = std::chrono::duration<float, std::chrono::milliseconds::period>(
                    currentTime - lastTime).count();

            camera.velocity += camera.direction * deltaTime;
            camera.velocity *= 0.1f;
            camera.position += camera.velocity * deltaTime;

            auto &mvpBuffer = mvpBuffers[currentFrame];
            mvpBuffer->view = camera.viewMatrix();
            mvpBuffer->proj = projection;

            auto &lightBuffer = lightBuffers[currentFrame];
            lightBuffer->eyePosition = camera.position;

            auto const &materialBuffer = materialBuffers[currentFrame];

            drawFrame(mvpBuffer, lightBuffer, materialBuffer, vertexBuffer, indexBuffer,
                      static_cast<std::uint32_t>(chunk.indices.size()), currentFrame);

            while (SDL_PollEvent(&event)) {
                eventHandler(event);
            }

            lastTime = currentTime;
        }

        device->waitIdle();
    }
};

int main(int argc, char **argv) {
    try {
        vkx::Window window{"Vulkan", WIDTH, HEIGHT};
        VoxelRenderer app{window};
        app.run();
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}