#include <vkx/application.hpp>

#include <iostream>

vkx::Application::Application() {
    std::cout << "Initializing Application\n";   
}

vkx::Application::~Application() {
    std::cout << "Destroying Application\n";
}