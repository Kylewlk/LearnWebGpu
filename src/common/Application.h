//
// Created by DELL on 2024/12/12.
//

#pragma once

#include <Windows.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <webgpu/webgpu_cpp.h>
#include "Utils.h"
#include "MathHelp.h"


class Application {
public:

    Application(bool canResize, std::string_view example);
    virtual ~Application();

    virtual void clean();
    virtual void pollEvent();
    virtual void render() = 0;

    [[nodiscard]] bool IsRunning() const { return !glfwWindowShouldClose(window); }
    [[nodiscard]] GLFWwindow* GetWindow() const { return window; }
    [[nodiscard]] HWND GetHWND() const { return glfwGetWin32Window(this->window); }
    [[nodiscard]] double getFps() const { return fps; }

    void setTitle(std::string_view title);
    void setTitleFps();
    void setSize(int width, int height) const { glfwSetWindowSize(this->window, width, height); }

    void inspectAdapter() const;
    void inspectDevice() const;

    [[nodiscard]] wgpu::TextureView getNextSurfaceTextureView() const;

protected:

    virtual void onResize(int width, int height);

    bool InitGLFW(bool canResize = false);
    bool initWebGPU();
    void loadShaderModule();

    GLFWwindow* window{};
    std::string title {"Learn WebGPU"};
    std::string exampleDir{};
    std::string exampleName{};
    int width{640};
    int height{480};
    double frameCount{0};
    double frameTime{0};
    double fps{0};

    wgpu::Instance instance{};
    wgpu::Adapter adapter{};
    wgpu::Device device{};
    wgpu::Queue queue{};
    wgpu::ShaderModule shaderModule{};
    wgpu::Surface surface;
    wgpu::TextureFormat surfaceFormat{wgpu::TextureFormat::BGRA8Unorm};

};



