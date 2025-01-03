//
// Created by DELL on 2024/12/11.
//

#include <iostream>

#include <webgpu/webgpu_cpp.h>
#include <dawn/native/DawnNative.h>
#include <dawn/webgpu_cpp_print.h>

int main()
{
    std::cout << "Hello Web GPU" << std::endl;

    wgpu::InstanceDescriptor instanceDescriptor{};
    instanceDescriptor.features.timedWaitAnyEnable = true;
    wgpu::Instance instance = wgpu::CreateInstance(&instanceDescriptor);
    if (instance == nullptr) {
        std::cerr << "Instance creation failed!\n";
        return EXIT_FAILURE;
    }
    // Synchronously request the adapter.
    wgpu::RequestAdapterOptions options = {};
#if defined(_WIN32)
    options.backendType = wgpu::BackendType::Vulkan;
#elif defined(__APPLE__)
    options.backendType = wgpu::BackendType::Metal;
#endif
    options.powerPreference = wgpu::PowerPreference::HighPerformance;
    wgpu::Adapter adapter;
    wgpu::RequestAdapterCallbackInfo callbackInfo = {};
    callbackInfo.nextInChain = nullptr;
    callbackInfo.mode = wgpu::CallbackMode::WaitAnyOnly;
    callbackInfo.callback = [](WGPURequestAdapterStatus status,
                               WGPUAdapter adapter, WGPUStringView message,
                               void *userdata) {
        if (status != WGPURequestAdapterStatus_Success) {
            std::cerr << "Failed to get an adapter:" << message.data << std::endl;
            return;
        }
        *static_cast<wgpu::Adapter *>(userdata) = wgpu::Adapter::Acquire(adapter);
    };
    callbackInfo.userdata = &adapter;
    instance.WaitAny(instance.RequestAdapter(&options, callbackInfo), UINT64_MAX);
    if (adapter == nullptr) {
        std::cerr << "RequestAdapter failed!\n";
        return EXIT_FAILURE;
    }

    // auto nativeInstance = std::make_unique<dawn::native::Instance>();
    // std::vector<dawn::native::Adapter> adapters = nativeInstance->EnumerateAdapters();
    // for (const auto& ap : adapters) {
    //     adapter = wgpu::Adapter(ap.Get());
    // }

    wgpu::DawnAdapterPropertiesPowerPreference power_props{};

    wgpu::AdapterInfo info{};
    info.nextInChain = &power_props;

    adapter.GetInfo(&info);
    std::cout << "VendorID: " << std::hex << info.vendorID << std::dec << "\n";
    std::cout << "Vendor: " << info.vendor << "\n";
    std::cout << "Architecture: " << info.architecture << "\n";
    std::cout << "DeviceID: " << std::hex << info.deviceID << std::dec << "\n";
    std::cout << "Name: " << info.device << "\n";
    std::cout << "Driver description: " << info.description << "\n";
    std::cout << "Backend: " << info.backendType << "\n";
    std::cout << "Adapter Type: " << info.adapterType << "\n";

    // adapter.GetFeatures()

    std::cout << std::endl;

    return 0;
}