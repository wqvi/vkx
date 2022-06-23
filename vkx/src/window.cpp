#include <window.hpp>

#include <SDL2/SDL_vulkan.h>
#include <iostream>

namespace vkx {
    static const struct StaticInstance {
        StaticInstance() {
            auto const &errorCallback = [](auto code, const auto *msg) {
                std::cerr << "[GLFW errored with the code " << code << "]\n";
                std::cerr << "[This is a placeholder message. Please find me in " << __FILE__ << "!]\n";
                std::cerr << msg << "\n";
            };
            glfwSetErrorCallback(errorCallback);
            if (!glfwInit())
                throw std::runtime_error("Failed to initialize GLFW.");
            if (!glfwVulkanSupported())
                throw std::runtime_error("Failed to find GLFW vulkan support.");
        }

        ~StaticInstance() {
            glfwTerminate();
        }
    } initialized;

    Window::Window(const char *title, std::uint32_t width, std::uint32_t height) {
//        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); /* No OpenGL API flag. */
//        glfwWindowHint(GLFW_VISIBLE, false);
//
//        internalHandle = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title, nullptr, nullptr);
//        if (!internalHandle)
//            throw std::runtime_error("Failed to create window.");
//
//        glfwSetInputMode(internalHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//
//        glfwSetWindowUserPointer(internalHandle, this);
//        glfwSetCursorPosCallback(internalHandle, cursorPosCallback);
//        glfwSetKeyCallback(internalHandle, keyboardCallback);
//        glfwSetFramebufferSizeCallback(internalHandle, framebufferResizeCallback);

        SDL_Init(SDL_INIT_EVERYTHING);
        internalHandle = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

    }

    Window::~Window() {
//        glfwDestroyWindow(internalHandle);
        SDL_DestroyWindow(internalHandle);
        SDL_Quit();
    }

    bool Window::isOpen() const {
//        return !glfwWindowShouldClose(internalHandle);
    }

    void Window::show() const {
//        glfwShowWindow(internalHandle);
//        glfwSetWindowShouldClose(internalHandle, false);
    }

    void Window::hide() const {
//        glfwHideWindow(internalHandle);
//        glfwSetWindowShouldClose(internalHandle, true);
    }

    vk::UniqueSurfaceKHR Window::createSurface(vk::UniqueInstance const &instance) const {
        VkSurfaceKHR cSurface = nullptr;
//        if (glfwCreateWindowSurface(*instance, internalHandle, nullptr, &cSurface) != VK_SUCCESS)
//            throw std::runtime_error("Failed to create window surface.");
        SDL_Vulkan_CreateSurface(internalHandle, *instance, &cSurface);

        return vk::UniqueSurfaceKHR(cSurface, *instance);
    }

    std::pair<std::uint32_t, std::uint32_t> Window::getSize() const {
        int width = 0;
        int height = 0;
//        glfwGetFramebufferSize(internalHandle, &width, &height);
        SDL_Vulkan_GetDrawableSize(internalHandle, &width, &height);
        return {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
    }

    void Window::pollEvents() {
//        glfwPollEvents();
    }

    void Window::cursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
        auto subject = static_cast<Window *>(glfwGetWindowUserPointer(window));

        static glm::vec2 previous(xpos, ypos);
        glm::vec2 relative(xpos - previous.x, previous.y - ypos);
        previous = glm::vec2(xpos, ypos);

        subject->MouseSubject::notify(MouseEvent{{xpos, ypos}, relative});
    }

    void Window::keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto subject = static_cast<Window *>(glfwGetWindowUserPointer(window));

        subject->KeyboardSubject::notify(KeyboardEvent{key, scancode, action, mods});
    }


    void
    Window::framebufferResizeCallback(GLFWwindow *window, [[maybe_unused]] int width, [[maybe_unused]] int height) {
        auto subject = static_cast<Window *>(glfwGetWindowUserPointer(window));

        subject->FramebufferResizedSubject::notify(FramebufferResizedEvent{width, height});
    }

}