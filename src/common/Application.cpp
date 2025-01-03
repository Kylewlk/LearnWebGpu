//
// Created by DELL on 2024/12/12.
//

#include "Application.h"
#include <dawn/webgpu_cpp_print.h>

#include <format>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

/// @example is the cpp file
Application::Application(bool canResize, std::string_view example)
{
    int separatorIndex{0};
    int dotIndex{0};
    for (auto i = static_cast<int>(example.size()) - 1; i >= 0; i--)
    {
        if (example[i] == '.')
        {
            dotIndex = i;
        }
        else if (example[i] == '\\' || example[i] == '/')
        {
            separatorIndex = i;
            break;
        }
    }

    this->exampleName = example.substr(separatorIndex + 1, dotIndex - separatorIndex - 1);
    if (!this->exampleName.empty())
    {
        this->title += '-';
        this->title += exampleName;
    }
    this->exampleDir = example.substr(0, separatorIndex);
    this->initGLFW(canResize);
    this->initWebGPU();
    this->initSurface();
    this->loadShaderModule();
}

Application::~Application()
{
    Application::clean();
}

bool Application::initGLFW(bool canResize) {

    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW error: " << error << ",  " << description << std::endl;
    });

    if (!glfwInit()) {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, canResize);
    this->window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window ,[](GLFWwindow* window, int width, int height) {
        auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        app->onResize(width, height);
    });

    return true;
}

bool Application::initWebGPU()
{
    using namespace wgpu;
    InstanceDescriptor instanceDescriptor{};
    instanceDescriptor.features.timedWaitAnyEnable = true;
    this->instance = CreateInstance(&instanceDescriptor);

    RequestAdapterOptions adapterOptions{};
#if defined(_WIN32)
    options.backendType = wgpu::BackendType::Vulkan;
#elif defined(__APPLE__)
    adapterOptions.backendType = wgpu::BackendType::Metal;
#endif
    adapterOptions.powerPreference = PowerPreference::HighPerformance;
    auto requestAdapterFuture =
        instance.RequestAdapter(&adapterOptions, CallbackMode::WaitAnyOnly,
                                [this](RequestAdapterStatus status, Adapter adapter, StringView message) {
                                    if (status == RequestAdapterStatus::Success)
                                    {
                                        this->adapter = std::move(adapter);
                                    }
                                    else
                                    {
                                        std::cerr << "Failed to request adapter: " << status << ", " << message
                                                  << std::endl;
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
                                                   std::cerr << "Failed to request device: " << status << ", "
                                                             << message << std::endl;
                                               }
                                           }),
                     UINT64_MAX);
    if (this->device == nullptr)
    {
        std::cerr << "Failed to request device" << std::endl;
        return false;
    }

    return true;
}

void Application::initSurface()
{
    using namespace wgpu;
    SurfaceDescriptor surfaceDescriptor{};

#if defined(_WIN32)
    SurfaceSourceWindowsHWND surfaceSourceWindowsHwnd{};
    surfaceSourceWindowsHwnd.hwnd = glfwGetWin32Window(window);
    surfaceSourceWindowsHwnd.hinstance = GetModuleHandleW(nullptr);
    surfaceDescriptor.nextInChain = &surfaceSourceWindowsHwnd;
#elif defined(__APPLE__)
    // SetupWindowAndGetSurfaceDescriptorCocoa defined in GLFWUtils_metal.mm
    std::unique_ptr<wgpu::ChainedStruct, void (*)(wgpu::ChainedStruct*)> SetupWindowAndGetSurfaceDescriptorCocoa(GLFWwindow* window);
    auto chainedDescriptor = SetupWindowAndGetSurfaceDescriptorCocoa(window);
    surfaceDescriptor.nextInChain = chainedDescriptor.get();
#endif

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

}

void Application::loadShaderModule()
{
    std::string shaderFile = this->exampleDir;
    shaderFile += '/';
    shaderFile += this->exampleName;
    shaderFile += ".wgsl";

    if (this->device == nullptr || !std::filesystem::exists(shaderFile))
    {
        return;
    }

    std::ifstream file(shaderFile, std::ios::binary);
    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string sourceCode(fileSize, '\0');
    file.read(sourceCode.data(), fileSize);
    file.close();

    this->shaderModule = createShaderModule(this->device, sourceCode);

    std::cout << "Loaded shader file: " << shaderFile << std::endl;
}

void Application::onResize(int width, int height)
{
    this->width = width;
    this->height = height;
}

void Application::clean() {

    this->shaderModule = nullptr;
    this->surface = nullptr;
    this->queue =  nullptr;
    this->device = nullptr;
    this->adapter = nullptr;
    this->instance = nullptr;

    if (this->window != nullptr)
    {
        glfwDestroyWindow(window);
        this->window = nullptr;

        glfwTerminate();
    }
}

wgpu::TextureView Application::getNextSurfaceTextureView() const
{
    using namespace wgpu;
    SurfaceTexture texture;
    this->surface.GetCurrentTexture(&texture);
    TextureViewDescriptor viewDescriptor;
    viewDescriptor.nextInChain = nullptr;
    viewDescriptor.label = "Surface texture view";
    viewDescriptor.format =surfaceFormat;
    viewDescriptor.dimension = TextureViewDimension::e2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = TextureAspect::All;
    viewDescriptor.usage = TextureUsage::RenderAttachment;

    auto textureView = texture.texture.CreateView(&viewDescriptor);
    // auto textureView = texture.texture.CreateView(); // viewDescriptor会根据texture的属性构建，但usage=None
    return textureView;
}

void Application::pollEvent() {
    glfwPollEvents();

    auto ctime = glfwGetTime();
    ++frameCount;
    if (ctime - frameTime > 1.0)
    {
        auto delay = ctime - frameTime;
        this->fps = static_cast<int>(std::round(frameCount / delay));
        this->frameTime = ctime;
        frameCount = 0;
    }
}

void Application::setTitle(std::string_view title)
{
    glfwSetWindowTitle(this->window, title.data());
    this->title = title;
}

void Application::setTitleFps()
{
    auto str =  std::format("{}-FPS: {}", this->title, std::round(this->fps));
    glfwSetWindowTitle(this->window, str.c_str());
}

void Application::inspectAdapter() const
{
    using namespace wgpu;
	SupportedLimits supportedLimits = {};
	supportedLimits.nextInChain = nullptr;
	bool success = adapter.GetLimits(&supportedLimits);
	if (success) {
		std::cout << "Adapter limits:" << std::endl;
		std::cout << " - maxTextureDimension1D: " << supportedLimits.limits.maxTextureDimension1D << std::endl;
		std::cout << " - maxTextureDimension2D: " << supportedLimits.limits.maxTextureDimension2D << std::endl;
		std::cout << " - maxTextureDimension3D: " << supportedLimits.limits.maxTextureDimension3D << std::endl;
		std::cout << " - maxTextureArrayLayers: " << supportedLimits.limits.maxTextureArrayLayers << std::endl;
	}

    // SupportedFeatures features{};
    // adapter.GetFeatures(&features);
	// std::cout << "Adapter features:" << std::endl;
	// std::cout << std::hex; // Write integers as hexadecimal to ease comparison with webgpu.h literals
	// for (int i = 0; i < features.featureCount; ++i) {
	// 	std::cout << " - 0x" << features.features[i]  << std::endl;
	// }
	// std::cout << std::dec; // Restore decimal numbers
	AdapterInfo properties = {};
	properties.nextInChain = nullptr;
    adapter.GetInfo(&properties);

	std::cout << "Adapter properties:" << std::endl;
	std::cout << " - vendorID: " << properties.vendorID << std::endl;
    std::cout << " - vendorName: " << properties.vendor << std::endl;
    std::cout << " - architecture: " << properties.architecture << std::endl;
	std::cout << " - deviceID: " << properties.deviceID << std::endl;
    std::cout << " - driverDescription: " << properties.description << std::endl;
	std::cout << " - adapterType: " << properties.adapterType << std::endl;
	std::cout << " - backendType:" << properties.backendType << std::endl;
}

void Application::inspectDevice() const
{
    using namespace wgpu;
    SupportedFeatures features{};
    device.GetFeatures(&features);

	std::cout << "Device features:" << std::endl;
	std::cout << std::hex;
	for (int i = 0; i < features.featureCount; ++i) {
		std::cout << " - 0x" << features.features[i] << std::endl;
	}
	std::cout << std::dec;

	SupportedLimits limits = {};
	limits.nextInChain = nullptr;
    if (device.GetLimits(&limits))
    {
		std::cout << "Device limits:" << std::endl;
		std::cout << " - maxTextureDimension1D: " << limits.limits.maxTextureDimension1D << std::endl;
		std::cout << " - maxTextureDimension2D: " << limits.limits.maxTextureDimension2D << std::endl;
		std::cout << " - maxTextureDimension3D: " << limits.limits.maxTextureDimension3D << std::endl;
		std::cout << " - maxTextureArrayLayers: " << limits.limits.maxTextureArrayLayers << std::endl;
		std::cout << " - maxBindGroups: " << limits.limits.maxBindGroups << std::endl;
		std::cout << " - maxDynamicUniformBuffersPerPipelineLayout: " << limits.limits.maxDynamicUniformBuffersPerPipelineLayout << std::endl;
		std::cout << " - maxDynamicStorageBuffersPerPipelineLayout: " << limits.limits.maxDynamicStorageBuffersPerPipelineLayout << std::endl;
		std::cout << " - maxSampledTexturesPerShaderStage: " << limits.limits.maxSampledTexturesPerShaderStage << std::endl;
		std::cout << " - maxSamplersPerShaderStage: " << limits.limits.maxSamplersPerShaderStage << std::endl;
		std::cout << " - maxStorageBuffersPerShaderStage: " << limits.limits.maxStorageBuffersPerShaderStage << std::endl;
		std::cout << " - maxStorageTexturesPerShaderStage: " << limits.limits.maxStorageTexturesPerShaderStage << std::endl;
		std::cout << " - maxUniformBuffersPerShaderStage: " << limits.limits.maxUniformBuffersPerShaderStage << std::endl;
		std::cout << " - maxUniformBufferBindingSize: " << limits.limits.maxUniformBufferBindingSize << std::endl;
		std::cout << " - maxStorageBufferBindingSize: " << limits.limits.maxStorageBufferBindingSize << std::endl;
		std::cout << " - minUniformBufferOffsetAlignment: " << limits.limits.minUniformBufferOffsetAlignment << std::endl;
		std::cout << " - minStorageBufferOffsetAlignment: " << limits.limits.minStorageBufferOffsetAlignment << std::endl;
		std::cout << " - maxVertexBuffers: " << limits.limits.maxVertexBuffers << std::endl;
		std::cout << " - maxVertexAttributes: " << limits.limits.maxVertexAttributes << std::endl;
		std::cout << " - maxVertexBufferArrayStride: " << limits.limits.maxVertexBufferArrayStride << std::endl;
		std::cout << " - maxInterStageShaderComponents: " << limits.limits.maxInterStageShaderComponents << std::endl;
		std::cout << " - maxComputeWorkgroupStorageSize: " << limits.limits.maxComputeWorkgroupStorageSize << std::endl;
		std::cout << " - maxComputeInvocationsPerWorkgroup: " << limits.limits.maxComputeInvocationsPerWorkgroup << std::endl;
		std::cout << " - maxComputeWorkgroupSizeX: " << limits.limits.maxComputeWorkgroupSizeX << std::endl;
		std::cout << " - maxComputeWorkgroupSizeY: " << limits.limits.maxComputeWorkgroupSizeY << std::endl;
		std::cout << " - maxComputeWorkgroupSizeZ: " << limits.limits.maxComputeWorkgroupSizeZ << std::endl;
		std::cout << " - maxComputeWorkgroupsPerDimension: " << limits.limits.maxComputeWorkgroupsPerDimension << std::endl;
	}
}
