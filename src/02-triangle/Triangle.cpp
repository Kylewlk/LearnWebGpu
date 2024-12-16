//
// Created by DELL on 2024/12/11.
//

#include <iostream>
#include <utility>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <string>
#include <GLFW/glfw3native.h>

#include <webgpu/webgpu_cpp.h>
#include <dawn/webgpu_cpp_print.h>

class TrianlgeApplication {
public:
    // Initialize everything and return true if it went all right
    bool Initialize()
    {
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

        initWebGPU();

        setup();

        return true;
    }

    // Uninitialize everything that was initialized
    void Terminate()
    {
        this->device = nullptr;
        this->adapter = nullptr;
        this->instance = nullptr;

        glfwDestroyWindow(window);
        this->window = nullptr;

        glfwTerminate();
    }

    // Draw a frame and handle events
    void MainLoop()
    {
        glfwPollEvents();

        using namespace wgpu;
        SurfaceTexture texture;
        this->surface.GetCurrentTexture(&texture);

        RenderPassColorAttachment colorAttachment{};
        colorAttachment.view = texture.texture.CreateView();
        colorAttachment.loadOp = LoadOp::Clear;
        colorAttachment.storeOp = StoreOp::Store;
        colorAttachment.clearValue = {0.2, 0.2, 0.2, 1.0};

        RenderPassDescriptor renderPassDescriptor{};
        renderPassDescriptor.colorAttachments = &colorAttachment;
        renderPassDescriptor.colorAttachmentCount = 1;

        auto encoder = device.CreateCommandEncoder();

        auto pass = encoder.BeginRenderPass(&renderPassDescriptor);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.Draw(3);
        pass.End();
        auto commands =  encoder.Finish();
        this->queue.Submit(1, &commands);

        this->surface.Present();

        auto ctime = glfwGetTime();
        ++frameCount;
        if (ctime - frameTime > 1.0)
        {
            auto delay = ctime - frameTime;
            auto fps = static_cast<int>(std::round(frameCount / delay));
            auto title = std::string("Learn WebGPU Triangle-FPS: ") + std::to_string(fps);
            glfwSetWindowTitle(window, title.c_str());
            this->frameTime = ctime;
            frameCount = 0;
        }
    }

    // Return true as long as the main loop should keep on running
    bool IsRunning()
    {
        return !glfwWindowShouldClose(window);
    }

    [[nodiscard]] GLFWwindow* GetWindow() const { return window; }

    [[nodiscard]] HWND GetHWND() const { return glfwGetWin32Window(this->window); }

    WGPUTextureView getNextSurfaceTextureView();


private:

    bool initWebGPU()
    {
        using namespace wgpu;
        InstanceDescriptor instanceDescriptor{};
        instanceDescriptor.features.timedWaitAnyEnable = true;
        this->instance = CreateInstance(&instanceDescriptor);

        RequestAdapterOptions adapterOptions{};
        adapterOptions.backendType = BackendType::Vulkan;
        adapterOptions.powerPreference = PowerPreference::HighPerformance;
        auto requestAdapterFuture = instance.RequestAdapter(
            &adapterOptions, CallbackMode::WaitAnyOnly,
            [this](RequestAdapterStatus status, Adapter adapter, StringView message) {
                if (status == RequestAdapterStatus::Success)
                {
                    this->adapter = std::move(adapter);
                }
                else
                {
                    std::cerr << "Failed to request adapter: " << status << ", " << message << std::endl;
                }
            });
        const auto waitStatus = this->instance.WaitAny(requestAdapterFuture, UINT64_MAX);
        std::cout << "RequestAdapterFuture: " << waitStatus << std::endl;

        if (this->adapter == nullptr)
        {
            std::cerr << "Failed to request adapter" << std::endl;
            return false;
        }

        AdapterInfo adapterInfo{};
        this->adapter.GetInfo(&adapterInfo);
        std::cout << "Using Adapter: " << adapterInfo.device << std::endl;
        std::cout << "Adapter Backend: " << adapterInfo.backendType << std::endl;

        DeviceDescriptor deviceDescriptor{};
        deviceDescriptor.SetDeviceLostCallback(CallbackMode::AllowSpontaneous,
            [this](const Device&, DeviceLostReason reason, StringView message) {
                std::cerr << "Device lost: " << reason << ", " << message;
            });
        deviceDescriptor.SetUncapturedErrorCallback([](const Device&, ErrorType type, StringView message) {
            std::cerr << "Device uncaptured Error: " << type << ", " << message << std::endl;
        });
        instance.WaitAny(adapter.RequestDevice(&deviceDescriptor, CallbackMode::WaitAnyOnly,
            [this](RequestDeviceStatus status, Device adapter, StringView message) {
                if (status == RequestDeviceStatus::Success)
                {
                    this->device = std::move(adapter);
                    this->queue = device.GetQueue();
                }
                else
                {
                    std::cerr << "Failed to request device: " << status << ", " << message << std::endl;
                }
            }), UINT64_MAX);
        if (this->device == nullptr)
        {
            std::cerr << "Failed to request device" << std::endl;
            return false;
        }

        SurfaceSourceWindowsHWND surfaceSourceWindowsHwnd{};
        surfaceSourceWindowsHwnd.hwnd = glfwGetWin32Window(window);
        surfaceSourceWindowsHwnd.hinstance = GetModuleHandleW(nullptr);
        SurfaceDescriptor surfaceDescriptor{};
        surfaceDescriptor.nextInChain = &surfaceSourceWindowsHwnd;
        this->surface = instance.CreateSurface(&surfaceDescriptor);

        SurfaceCapabilities capabilities;
        surface.GetCapabilities(adapter, &capabilities);
        this->surfaceFormat = capabilities.formats[0];
        SurfaceConfiguration config = {};
        config.device = device;
        config.format = surfaceFormat;
        config.presentMode = PresentMode::Fifo;
        int width{640}, height{480};
        glfwGetWindowSize(this->window, &width, &height);
        config.width = width;
        config.height = height;
        surface.Configure(&config);
        std::cout << "Surface size: " << width << ", " << height << std::endl;
        std::cout << "Surface format: " << config.format << std::endl;

        return true;
    }

    bool setup()
    {
        using namespace wgpu;
        static const float vertexData[12] = {
            0.0f, 0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.0f, 1.0f,
        };
        BufferDescriptor vertexBufferDescriptor{};
        vertexBufferDescriptor.usage = BufferUsage::CopyDst | BufferUsage::Vertex;
        vertexBufferDescriptor.size = sizeof(vertexData);
        vertexBuffer = this->device.CreateBuffer(&vertexBufferDescriptor);
        queue.WriteBuffer(vertexBuffer, 0, vertexData, sizeof(vertexData));

        constexpr const char* shaderCode = R"(
        @vertex fn vs(@location(0) pos : vec4f) -> @builtin(position) vec4f {
            return pos;
        }

        @fragment fn fs(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
            return vec4f(1, 0, 0, 1);
        }
        )";
        ShaderModuleWGSLDescriptor shaderCodeDescriptor{};
        shaderCodeDescriptor.code = shaderCode;
        ShaderModuleDescriptor shaderDescriptor{};
        shaderDescriptor.nextInChain = &shaderCodeDescriptor;
        auto shaderModule = this->device.CreateShaderModule(&shaderDescriptor);

        VertexAttribute positionAttrib{};
        positionAttrib.shaderLocation = 0;
        positionAttrib.format = VertexFormat::Float32x4;
        positionAttrib.offset = 0;
        VertexBufferLayout vertexBufferLayout{};
        vertexBufferLayout.attributeCount = 1;
        vertexBufferLayout.attributes = &positionAttrib;
        vertexBufferLayout.stepMode = VertexStepMode::Vertex;
        vertexBufferLayout.arrayStride = sizeof(float) * 4;

        RenderPipelineDescriptor descriptor{};
        descriptor.vertex.bufferCount = 1;
        descriptor.vertex.buffers = &vertexBufferLayout;

        descriptor.vertex.module = shaderModule;
        descriptor.vertex.entryPoint = "vs";
        descriptor.vertex.constantCount = 0;
        descriptor.vertex.constants = nullptr;

        descriptor.primitive.topology = PrimitiveTopology::TriangleList;
        descriptor.primitive.stripIndexFormat = IndexFormat::Undefined;
        descriptor.primitive.frontFace = FrontFace::CW;
        descriptor.primitive.cullMode = CullMode::None;

        FragmentState fragmentState;
        fragmentState.module = shaderModule;
        fragmentState.entryPoint = "fs";
        fragmentState.constantCount = 0;
        fragmentState.constants = nullptr;

        BlendState blendState;
        blendState.color.srcFactor = BlendFactor::SrcAlpha;
        blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
        blendState.color.operation = BlendOperation::Add;
        blendState.alpha.srcFactor = BlendFactor::Zero;
        blendState.alpha.dstFactor = BlendFactor::One;
        blendState.alpha.operation = BlendOperation::Add;

        ColorTargetState colorTarget;
        colorTarget.format = surfaceFormat;
        colorTarget.blend = &blendState;
        colorTarget.writeMask = ColorWriteMask::All; // We could write to only some of the color channels.

        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;
        descriptor.fragment = &fragmentState;

        descriptor.depthStencil = nullptr;

        descriptor.multisample.count = 1;
        descriptor.multisample.mask = ~0u;
        descriptor.multisample.alphaToCoverageEnabled = false;

        descriptor.layout = nullptr;

        pipeline = device.CreateRenderPipeline(&descriptor);

        return true;
    }

    GLFWwindow* window{};

    wgpu::Instance instance{};
    wgpu::Adapter adapter{};
    wgpu::Device device{};
    wgpu::Queue queue{};
    wgpu::Surface surface;
    wgpu::TextureFormat surfaceFormat{wgpu::TextureFormat::BGRA8Unorm};

    wgpu::Buffer vertexBuffer{};
    wgpu::RenderPipeline pipeline{};

    double frameCount{0};
    double frameTime{0};
};




int main()
{
    TrianlgeApplication app;

    if (!app.Initialize()) {
        return 1;
    }

    // Warning: this is still not Emscripten-friendly, see below
    while (app.IsRunning()) {
        app.MainLoop();
    }

    app.Terminate();

    return 0;
}