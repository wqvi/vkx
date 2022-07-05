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
        camera.velocity += camera.direction * deltaTime;
        camera.velocity *= 0.1f;
        camera.position += camera.velocity * deltaTime;

        mvp.view = camera.viewMatrix();
        directionalLight.eyePosition = camera.position;
    }

    void destroy() noexcept override {
        // Temporarily empty
    }

    void onKeyPress(const SDL_KeyboardEvent &event) override {
        // Temporarily empty
        camera.updateKey(event.keysym.sym);
    }

    void onKeyRelease(const SDL_KeyboardEvent &event) override {
        // Temporarily empty
        camera.updateKey(0);
    }

    void onMouseMove(const SDL_MouseMotionEvent &event) override {
        // Temporarily empty
        camera.updateMouse(glm::vec2{event.xrel, -event.yrel});
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
    vkx::Camera camera{{2.0f, 2.0f, 2.0f}};

    vkx::MVP mvp = {};
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