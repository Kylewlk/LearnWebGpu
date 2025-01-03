//
// Created by DELL on 2024/12/13.
//

#pragma once

#include <webgpu/webgpu_cpp.h>

wgpu::ShaderModule createShaderModule(const wgpu::Device& device, std::string_view source);

template <typename T, typename U,  typename = std::enable_if_t<std::is_integral_v<T> && std::is_integral_v<U>>>
constexpr T ceilToNextMultiple(T value, U step)
{
    auto count = (value + (step - 1)) / static_cast<T>(step);
    return count * static_cast<T>(step);
}
