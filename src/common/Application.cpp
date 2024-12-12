//
// Created by DELL on 2024/12/12.
//

#include "Application.h"

#include <iostream>

bool Application::Initialize() {
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW error: " << error << ",  " << description << std::endl;
    });

    if (!glfwInit()) {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // <-- extra info for glfwCreateWindow
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    this->window = glfwCreateWindow(640, 480, "Learn WebGPU", nullptr, nullptr);
    if (!window) {
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return false;
    }

    return true;
}

void Application::Terminate() {

    glfwDestroyWindow(window);
    this->window = nullptr;

    glfwTerminate();
}

void Application::MainLoop() {
    glfwPollEvents();

    // [...] Main loop content

    // Also move here the tick/poll but NOT the emscripten sleep
    // [...] Poll WebGPU Events
}

bool Application::IsRunning() {
    return !glfwWindowShouldClose(window);
}