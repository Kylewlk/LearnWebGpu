//
// Created by DELL on 2024/12/12.
//

#include "Application.h"

#include <iostream>
#include <webgpu/webgpu_cpp.h>

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

    initWebGPU();



    return true;
}

void Application::Terminate() {

    wgpuSurfaceUnconfigure(surface);
    wgpuQueueRelease(queue);
    wgpuSurfaceRelease(surface);
    wgpuDeviceRelease(device);

    wgpuAdapterRelease(this->adapter);
    wgpuInstanceRelease(this->instance);

    glfwDestroyWindow(window);
    this->window = nullptr;

    glfwTerminate();
}

void Application::MainLoop() {
    glfwPollEvents();

// Get the next target texture view
	WGPUTextureView targetView = getNextSurfaceTextureView();
	if (!targetView) return;

	// Create a command encoder for the draw call
	WGPUCommandEncoderDescriptor encoderDesc = {};
	encoderDesc.nextInChain = nullptr;
	encoderDesc.label = WGPUStringView{"My command encoder", WGPU_STRLEN};
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

	// Create the render pass that clears the screen with our color
	WGPURenderPassDescriptor renderPassDesc = {};
	renderPassDesc.nextInChain = nullptr;

	// The attachment part of the render pass descriptor describes the target texture of the pass
	WGPURenderPassColorAttachment renderPassColorAttachment = {};
	renderPassColorAttachment.view = targetView;
	renderPassColorAttachment.resolveTarget = nullptr;
	renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
	renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
	renderPassColorAttachment.clearValue = WGPUColor{ 0.9, 0.1, 0.2, 1.0 };
#ifndef WEBGPU_BACKEND_WGPU
	renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU

	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &renderPassColorAttachment;
	renderPassDesc.depthStencilAttachment = nullptr;
	renderPassDesc.timestampWrites = nullptr;

	// Create the render pass and end it immediately (we only clear the screen but do not draw anything)
	WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
	wgpuRenderPassEncoderEnd(renderPass);
	wgpuRenderPassEncoderRelease(renderPass);

	// Finally encode and submit the render pass
	WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
	cmdBufferDescriptor.nextInChain = nullptr;
	cmdBufferDescriptor.label = WGPUStringView{"Command buffer", WGPU_STRLEN};
	WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
	wgpuCommandEncoderRelease(encoder);

	std::cout << "Submitting command..." << std::endl;
	wgpuQueueSubmit(queue, 1, &command);
	wgpuCommandBufferRelease(command);
	std::cout << "Command submitted." << std::endl;

	// At the end of the frame
	wgpuTextureViewRelease(targetView);
#ifndef __EMSCRIPTEN__
	wgpuSurfacePresent(surface);
#endif
}

bool Application::IsRunning() {
    return !glfwWindowShouldClose(window);
}

void Application::initWebGPU()
{
    // create instance
    instance = wgpuCreateInstance(nullptr);

    // init surface
    HWND hwnd = glfwGetWin32Window(window);
    HINSTANCE hinstance = GetModuleHandleW(nullptr);
    WGPUSurfaceDescriptorFromWindowsHWND fromWindowsHWND{};
    fromWindowsHWND.chain.next = nullptr;
    fromWindowsHWND.chain.sType = WGPUSType_SurfaceSourceWindowsHWND;
    fromWindowsHWND.hinstance = hinstance;
    fromWindowsHWND.hwnd = hwnd;
    WGPUSurfaceDescriptor surfaceDescriptor{};
    surfaceDescriptor.nextInChain = &fromWindowsHWND.chain;
    this->surface = wgpuInstanceCreateSurface(instance, &surfaceDescriptor);

    // adapter
    std::cout << "Requesting adapter..." << std::endl;
    WGPURequestAdapterOptions adapterOpts = {};
    adapterOpts.nextInChain = nullptr;
    adapterOpts.compatibleSurface = surface;
    adapterOpts.backendType = WGPUBackendType::WGPUBackendType_Vulkan;
    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void * pUserData) {
        auto app = reinterpret_cast<Application*>(pUserData);
        if (status == WGPURequestAdapterStatus_Success) {
            app->adapter = adapter;
        } else {
            std::cerr << "Could not get WebGPU adapter: " << message.data << std::endl;
        }
    };
    wgpuInstanceRequestAdapter( instance, &adapterOpts, onAdapterRequestEnded, this );
    std::cout << "Got adapter: " << adapter << std::endl;

    // device
    std::cout << "Requesting device..." << std::endl;
    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = nullptr;
    deviceDesc.label = WGPUStringView{"My Device", WGPU_STRLEN};
    deviceDesc.requiredFeatureCount = 0;
    deviceDesc.requiredLimits = nullptr;
    deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.defaultQueue.label =  WGPUStringView{"The default queue", WGPU_STRLEN};
    deviceDesc.deviceLostCallbackInfo2.callback = [](const WGPUDevice * device, WGPUDeviceLostReason reason, WGPUStringView message, void* userdata1, void* userdata2 ) {
        std::cout << "Device lost: reason " << reason;
        if (message.data)
            std::cout << " (" << message.data << ")";
        std::cout << std::endl;
    };
    deviceDesc.deviceLostCallbackInfo2.mode = WGPUCallbackMode_WaitAnyOnly;
    deviceDesc.uncapturedErrorCallbackInfo2.callback = [](const WGPUDevice * device, WGPUErrorType type, WGPUStringView message, void* userdata1, void* userdata2) {
        std::cout << "Uncaptured device error: type " << type;
        if (message.data)
            std::cout << " (" << message.data << ")";
        std::cout << std::endl;
    };
    auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void * pUserData) {
        auto app = reinterpret_cast<Application*>(pUserData);
        if (status == WGPURequestDeviceStatus_Success) {
            app->device = device;
        } else {
            std::cout << "Could not get WebGPU device: " << message.data << std::endl;
        }
    };
    wgpuAdapterRequestDevice(adapter, &deviceDesc, onDeviceRequestEnded, this);
    std::cout << "Got device: " << device << std::endl;


    this->queue = wgpuDeviceGetQueue(this->device);

    // Configure the surface
    WGPUSurfaceConfiguration config = {};
    config.nextInChain = nullptr;

    // Configuration of the textures created for the underlying swap chain
    WGPUSurfaceCapabilities capabilities;
    wgpuSurfaceGetCapabilities(surface, adapter, &capabilities);
    config.width = 640;
    config.height = 480;
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.format = capabilities.formats[0];
    wgpuSurfaceCapabilitiesFreeMembers(capabilities);

    // And we do not need any particular view format:
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.device = device;
    config.presentMode = WGPUPresentMode_Fifo;
    config.alphaMode = WGPUCompositeAlphaMode_Auto;

    wgpuSurfaceConfigure(surface, &config);

}

void Application::inspectAdapter() const
{
	WGPUSupportedLimits supportedLimits = {};
	supportedLimits.nextInChain = nullptr;
	bool success = wgpuAdapterGetLimits(adapter, &supportedLimits);
	if (success) {
		std::cout << "Adapter limits:" << std::endl;
		std::cout << " - maxTextureDimension1D: " << supportedLimits.limits.maxTextureDimension1D << std::endl;
		std::cout << " - maxTextureDimension2D: " << supportedLimits.limits.maxTextureDimension2D << std::endl;
		std::cout << " - maxTextureDimension3D: " << supportedLimits.limits.maxTextureDimension3D << std::endl;
		std::cout << " - maxTextureArrayLayers: " << supportedLimits.limits.maxTextureArrayLayers << std::endl;
	}

    WGPUSupportedFeatures features{};
	wgpuAdapterGetFeatures(adapter, &features);

	// std::cout << "Adapter features:" << std::endl;
	// std::cout << std::hex; // Write integers as hexadecimal to ease comparison with webgpu.h literals
	// for (int i = 0; i < features.featureCount; ++i) {
	// 	std::cout << " - 0x" << features.features[i]  << std::endl;
	// }
 //    wgpuSupportedFeaturesFreeMembers(features);
	// std::cout << std::dec; // Restore decimal numbers
	WGPUAdapterInfo properties = {};
	properties.nextInChain = nullptr;
	wgpuAdapterGetInfo(adapter, &properties);
	std::cout << "Adapter properties:" << std::endl;
	std::cout << " - vendorID: " << properties.vendorID << std::endl;
	if (properties.vendor.data) {
		std::cout << " - vendorName: " << properties.vendor.data << std::endl;
	}
	if (properties.architecture.data) {
		std::cout << " - architecture: " << properties.architecture.data << std::endl;
	}
	std::cout << " - deviceID: " << properties.deviceID << std::endl;
	if (properties.description.data) {
		std::cout << " - driverDescription: " << properties.description.data << std::endl;
	}
	std::cout << std::hex;
	std::cout << " - adapterType: 0x" << properties.adapterType << std::endl;
	std::cout << " - backendType: 0x" << properties.backendType << std::endl;
	std::cout << std::dec; // Restore decimal numbers
}

void Application::inspectDevice() const
{
    WGPUSupportedFeatures features{};
    wgpuDeviceGetFeatures(device, &features);
	std::cout << "Device features:" << std::endl;
	std::cout << std::hex;
	for (int i = 0; i < features.featureCount; ++i) {
		std::cout << " - 0x" << features.features[i] << std::endl;
	}
	std::cout << std::dec;
    wgpuSupportedFeaturesFreeMembers(features);

	WGPUSupportedLimits limits = {};
	limits.nextInChain = nullptr;
	bool success = wgpuDeviceGetLimits(device, &limits);
	if (success) {
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

WGPUTextureView Application::getNextSurfaceTextureView()
{
    // Get the surface texture
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
        return nullptr;
    }

    // Create a view for this surface texture
    WGPUTextureViewDescriptor viewDescriptor;
    viewDescriptor.nextInChain = nullptr;
    viewDescriptor.label = WGPUStringView{"Surface texture view", WGPU_STRLEN};
    viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
    viewDescriptor.dimension = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = WGPUTextureAspect_All;
    viewDescriptor.usage = WGPUTextureUsage_RenderAttachment;
    WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

#ifndef WEBGPU_BACKEND_WGPU
    // We no longer need the texture, only its view
    // (NB: with wgpu-native, surface textures must not be manually released)
    wgpuTextureRelease(surfaceTexture.texture);
#endif // WEBGPU_BACKEND_WGPU

    return targetView;
}