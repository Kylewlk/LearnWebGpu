//
// Created by DELL on 2024/12/13.
//

#pragma once

#include <webgpu/webgpu_cpp.h>

wgpu::ShaderModule createShaderModule(const wgpu::Device& device, std::string_view source);

