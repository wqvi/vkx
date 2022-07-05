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
        directionalLight = {
                glm::vec3(1.0f, 3.0f, 1.0f),
                getCamera().position,
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

        data->model = &model;
    }

    void update() override {
        // Temporarily empty
    }


    void physics(float deltaTime) override {
        // Temporarily empty
        getCamera().velocity += getCamera().direction * deltaTime;
        getCamera().velocity *= 0.1f;
        getCamera().position += getCamera().velocity * deltaTime;

        directionalLight.eyePosition = getCamera().position;
    }

    void destroy() noexcept override {
        // Temporarily empty
    }

    void onKeyPress(const SDL_KeyboardEvent &event) override {
        // Temporarily empty
        getCamera().updateKey(event.keysym.sym);
    }

    void onKeyRelease(const SDL_KeyboardEvent &event) override {
        // Temporarily empty
        getCamera().updateKey(0);
    }

    void onMouseMove(const SDL_MouseMotionEvent &event) override {
        // Temporarily empty
        getCamera().updateMouse(glm::vec2{event.xrel, -event.yrel});
    }

    void onWindowResize(Sint32 width, Sint32 height) override {
        // Temporarily empty
    }

private:
    vkx::DirectionalLight directionalLight = {};
    vkx::Material material = {};

    vkx::Model model;

    vkx::VoxelChunk chunk{glm::vec3(0), 16, 15, 14};
};

int main(int argc, char **argv) {
    try {
        vkx::ApplicationConfig appConfig = {"VKX App", WIDTH, HEIGHT};
        auto myApplication = MyApplication{appConfig};
        myApplication.setScene(new MyScene{});
        myApplication.run();
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}