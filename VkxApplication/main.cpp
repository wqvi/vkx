//
// Created by december on 6/21/22.
//

#include <vkx.hpp>
#include <iostream>

const std::uint32_t WIDTH = 800;
const std::uint32_t HEIGHT = 600;

class MyApplication : public vkx::Application {
public:
    using Application::Application;
};

class MyScene : public vkx::Scene {
public:
    MyScene() = default;

    ~MyScene() override = default;

    void init(const vkx::ApplicationConfig *config,
              vkx::Application *data,
              const vkx::RendererBase &rendererState) override {
        windowProjection = glm::perspective(glm::radians(75.0f), static_cast<float>(config->windowWidth) /
                                                                 static_cast<float>(config->windowHeight), 0.1f,
                                            100.0f);
        windowProjection[1][1] *= -1.0f;

        mvp = {glm::mat4(1.0f), camera.viewMatrix(), windowProjection};

        directionalLight = {
                glm::vec3(1.0f, 3.0f, 1.0f),
                camera.position,
                glm::vec4(1.0f, 1.0f, 1.0f, 0.2f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                1.0f,
                0.09f,
                0.032f
        };

        material = {
                glm::vec3(0.2f, 0.2f, 0.2f),
                100.0f
        };

        chunk.greedy();

        model = vkx::Model{
                rendererState.allocateMesh(chunk.vertices, chunk.indices),
                rendererState.allocateTexture("a.jpg"),
                {glm::vec3(0.2f), 100.0f}
        };

        data->windowProjection = &windowProjection;
        data->camera = &camera;
        data->model = &model;
    }

    void update() override {
        // Temporarily empty
    }


    void physics(float deltaTime) override {
        // Temporarily empty
    }

    void destroy() noexcept override {
        // Temporarily empty
    }

    void onKeyPress() override {
        // Temporarily empty
    }

    void onKeyRelease() override {
        // Temporarily empty
    }

    void onMouseMove() override {
        // Temporarily empty
    }

    void onWindowResize(Sint32 width, Sint32 height) override {
        // Temporarily empty
        windowProjection = glm::perspective(glm::radians(75.0f), static_cast<float>(width) / static_cast<float>(height),
                                            0.1f, 100.0f);
        windowProjection[1][1] *= -1.0f;
    }

private:
    // TODO Move me to the application instead and have the mvp buffer uploaded another way
    glm::mat4 windowProjection = glm::mat4(1);

    // TODO Move my matrix uploading else where similar to the window projection the scene does NOT need to manually upload it
    Camera camera{{2.0f, 2.0f, 2.0f}};

    vkx::MVP mvp = {};
    vkx::DirectionalLight directionalLight = {};
    vkx::Material material = {};

    vkx::Model model;

    vkx::VoxelChunk chunk{glm::vec3(0), 16, 15, 14};
};

class VoxelRenderer : private vkx::RendererBase {
public:
    explicit VoxelRenderer(vkx::SDLWindow &window)
            : vkx::RendererBase(window, vkx::Profile{}) {}

    void run() {
        initVulkan();
        mainLoop();
    }

    Camera camera{{2.0f, 2.0f, 2.0f}};

    vkx::Texture texture;

    vkx::Buffer vertexBuffer;
    vkx::Buffer indexBuffer;

    vkx::Mesh mesh;

    std::vector<vkx::UniformBuffer<vkx::MVP>> mvpBuffers;
    std::vector<vkx::UniformBuffer<vkx::DirectionalLight>> lightBuffers;
    std::vector<vkx::UniformBuffer<vkx::Material>> materialBuffers;

    vkx::VoxelChunk chunk{glm::vec3(0), 16, 15, 14};

    glm::mat4 projection = glm::mat4(1.0f);

    void initVulkan() {
        chunk.greedy();

        texture = vkx::Texture{"a.jpg", device};

        vertexBuffer = vkx::Buffer{chunk.vertices, device};
        indexBuffer = vkx::Buffer{chunk.indices, device};

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
                window->setFramebufferResized(true);
                projection = glm::perspective(glm::radians(75.0f),
                                              static_cast<float>(event.data1) / static_cast<float>(event.data2), 0.1f,
                                              100.0f);
                projection[1][1] *= -1.0f;
                break;
            default:
                return;
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
            case SDL_CONTROLLERAXISMOTION:
                std::cout << "Hello World!\n";
                break;
            default:
                break;
        }
    }

    void mainLoop() {
        window->show();
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
        vkx::ApplicationConfig appConfig = {"VKX App", WIDTH, HEIGHT};
        auto myApplication = MyApplication{appConfig};
        myApplication.setScene(new MyScene{});
//        vkx::SDLWindow window{"Vulkan", WIDTH, HEIGHT};
//        VoxelRenderer app{window};
//        app.run();
        myApplication.run();
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}