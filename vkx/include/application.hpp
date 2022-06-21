//
// Created by december on 6/21/22.
//

#pragma once

namespace vkx {
    struct AppConfig {
        std::uint32_t windowWidth;
        std::uint32_t windowHeight;
    };

    class App {
    public:
        App() = default;

        App(const AppConfig &configuration);

        ~App() = default;

        void run();
    };
}