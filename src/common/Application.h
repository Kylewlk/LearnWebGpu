//
// Created by DELL on 2024/12/12.
//

#pragma once

#include <Windows.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <webgpu/webgpu.h>



class Application {
public:
    // Initialize everything and return true if it went all right
    bool Initialize();

    // Uninitialize everything that was initialized
    void Terminate();

    // Draw a frame and handle events
    void MainLoop();

    // Return true as long as the main loop should keep on running
    bool IsRunning();

    [[nodiscard]] GLFWwindow* GetWindow() const { return window; }

    [[nodiscard]] HWND GetHWND() const { return glfwGetWin32Window(this->window); }

    void inspectAdapter() const;
    void inspectDevice() const;

    WGPUTextureView getNextSurfaceTextureView();


private:

    void initWebGPU();

    GLFWwindow* window{};

    WGPUInstance instance{};
    WGPUAdapter adapter{};
    WGPUDevice device{};
    WGPUQueue queue{};
    WGPUSurface surface{};
};



