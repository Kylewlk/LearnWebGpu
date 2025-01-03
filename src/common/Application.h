//
// Created by DELL on 2024/12/12.
//

#pragma once
#if defined(_WIN32)
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3.h>
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

    [[nodiscard]] bool isRunning() const { return !glfwWindowShouldClose(window); }
    [[nodiscard]] GLFWwindow* getWindow() const { return window; }

#if defined(_WIN32)
   [[nodiscard]] HWND GetHWND() const { return glfwGetWin32Window(this->window); }
#elif defined(__APPLE__)
    [[nodiscard]] id getNativeWindow() const { return glfwGetCocoaWindow(this->window); }
#endif

     [[nodiscard]] double getFps() const { return fps; }

    void setTitle(std::string_view title);
    void setTitleFps();
    void setSize(int width, int height) const { glfwSetWindowSize(this->window, width, height); }

    void inspectAdapter() const;
    void inspectDevice() const;

    [[nodiscard]] wgpu::TextureView getNextSurfaceTextureView() const;

protected:

    virtual void onResize(int width, int height);

    bool initGLFW(bool canResize = false);
    bool initWebGPU();
    void initSurface();
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
    wgpu::Limits deviceLimits{};

};



