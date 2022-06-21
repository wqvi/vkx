#pragma once

#include <util/observer.hpp>

namespace vkx {
    class Window : public MouseSubject, public KeyboardSubject, public FramebufferResizedSubject {
    public:
        Window(const char *title, std::uint32_t width, std::uint32_t height);

        ~Window() override;

        [[nodiscard]] bool isOpen() const;

        void show() const;

        void hide() const;

        [[nodiscard]] vk::UniqueSurfaceKHR createSurface(vk::UniqueInstance const &instance) const;

        [[nodiscard]] std::pair<std::uint32_t, std::uint32_t> getSize() const;

        static void pollEvents();

    private:
        GLFWwindow *internalHandle;

        static void cursorPosCallback(GLFWwindow *window, double xpos, double ypos);

        static void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

        static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    };
}