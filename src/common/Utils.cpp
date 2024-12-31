//
// Created by DELL on 2024/12/16.
//

#include "Utils.h"

wgpu::ShaderModule createShaderModule(const wgpu::Device& device, std::string_view source)
{
    using namespace wgpu;

    ShaderModuleWGSLDescriptor shaderCodeDescriptor{};
    shaderCodeDescriptor.code = source;
    ShaderModuleDescriptor shaderDescriptor{};
    shaderDescriptor.nextInChain = &shaderCodeDescriptor;
    auto shaderModule = device.CreateShaderModule(&shaderDescriptor);

    return shaderModule;
}