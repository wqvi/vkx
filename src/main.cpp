//
// Created by december on 6/21/22.
//

#include <vkx/vkx.hpp>
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

    void init(const vkx::GlobalConfiguration *config,
              vkx::Application *data,
              const vkx::RendererBase &rendererState) override {
        chunk.greedy();

        model = vkx::Model{
                rendererState.allocateMesh(chunk.vertices, chunk.indices),
                rendererState.allocateTexture("a.jpg"),
                {glm::vec3(0.2f), 100.0f}
        };

        data->model = &model;
    }

    void update() override {
        // Temporarily empty
    }


    void physics(float deltaTime) override {
        getCamera().velocity += getCamera().direction * deltaTime;
        getCamera().velocity *= 0.1f;
        getCamera().position += getCamera().velocity * deltaTime;
    }

    void destroy() noexcept override {
        // Temporarily empty
    }

    void onKeyPress(const SDL_KeyboardEvent &event) override {
        getCamera().updateKey(event.keysym.sym);
    }

    void onKeyRelease(const SDL_KeyboardEvent &event) override {
        getCamera().updateKey(0);
    }

    void onMouseMove(const SDL_MouseMotionEvent &event) override {
        getCamera().updateMouse(glm::vec2{event.xrel, -event.yrel});
    }

    void onWindowResize(Sint32 width, Sint32 height) override {
        // Temporarily empty
    }

private:
    vkx::Model model;

    vkx::VoxelChunk chunk{glm::vec3(0), 16, 15, 14};
};

int main(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_LogCritical(0, "Failure to initialize SDL2: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    auto windowFlags = 
        SDL_WINDOW_HIDDEN |
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_VULKAN;

    SDL_Window* window = SDL_CreateWindow(
        "Jewelry", 
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640, 
        480,
        windowFlags
        );

    if (window == nullptr) {
        SDL_LogCritical(0, "Failure to initialize SDL2 window: %s", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    vkx::RendererBase renderer{window};

    vkx::Model model{};

    vkx::VoxelChunk chunk{glm::vec3(0), 16, 15, 14};
    chunk.greedy();

    model = vkx::Model{
        renderer.allocateMesh(chunk.vertices, chunk.indices),
        renderer.allocateTexture("a.jpg"),
        {glm::vec3(0.2f), 100.0f}
    };

    vkx::Camera camera;
    vkx::Viewport viewport;

    std::vector<vkx::UniformBuffer<vkx::MVP>> mvpBuffers;
    std::vector<vkx::UniformBuffer<vkx::DirectionalLight>> lightBuffers;
    std::vector<vkx::UniformBuffer<vkx::Material>> materialBuffers;

    mvpBuffers = renderer.createBuffers(vkx::MVP{});
    lightBuffers = renderer.createBuffers(vkx::DirectionalLight{});
    materialBuffers = renderer.createBuffers(vkx::Material{});
    renderer.createDescriptorSets(mvpBuffers, lightBuffers, materialBuffers, model.texture);
    viewport.setSize(640, 480);

    SDL_Event event{};
    bool isRunning = true;
    SDL_ShowWindow(window);
    while (isRunning) {
        auto currentFrame = renderer.getCurrentFrameIndex();

        auto &mvpBuffer = mvpBuffers[currentFrame];
        mvpBuffer->model = model.getModelMatrix();
        mvpBuffer->view = camera.viewMatrix();
        mvpBuffer->proj = static_cast<glm::mat4>(viewport);

        auto &lightBuffer = lightBuffers[currentFrame];
        lightBuffer->position = glm::vec3(1.0f, 3.0f, 1.0f);
        lightBuffer->eyePosition = camera.position;
        lightBuffer->ambientColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.2f);
        lightBuffer->diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
        lightBuffer->specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
        lightBuffer->constant = 1.0f;
        lightBuffer->linear = 0.09f;
        lightBuffer->quadratic = 0.032f;

        auto &materialBuffer = materialBuffers[currentFrame];
        materialBuffer->specularColor = model.material.specularColor;
        materialBuffer->shininess = model.material.shininess;

        // TODO whatever this is lol
        renderer.drawFrame(mvpBuffer,
                           lightBuffer,
                           materialBuffer,
                           model.mesh.vertexBuffer,
                           model.mesh.indexBuffer,
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
                        viewport.setSize(event.window.data1, event.window.data2);
                    }
                    break;
                case SDL_KEYDOWN:
                    break;
                case SDL_KEYUP:
                    break;
                case SDL_MOUSEMOTION:
                    break;
            }
        }
    }

    renderer.waitIdle();

    /*try {
        vkx::GlobalConfiguration appConfig = {"VKX App", WIDTH, HEIGHT};
        auto myApplication = MyApplication{appConfig};
        myApplication.setScene(new MyScene{});
        myApplication.run();
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }*/

    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
