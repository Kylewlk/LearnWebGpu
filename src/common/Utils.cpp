//
// Created by DELL on 2024/12/16.
//

#include "Utils.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <glm/ext/scalar_uint_sized.hpp>
#include <stb/stb_image_write.h>

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

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

wgpu::Texture loadTexture(const wgpu::Device& device, const wgpu::Queue& queue, std::string_view filename)
{
    int width{}, height{}, channels{};
    auto imageData = stbi_load(filename.data(), &width, &height, &channels, STBI_rgb_alpha);
    using namespace wgpu;

    TextureDescriptor textureDesc{};
    textureDesc.dimension = TextureDimension::e2D;
    textureDesc.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.format = TextureFormat::RGBA8Unorm;
    textureDesc.usage = TextureUsage::TextureBinding | TextureUsage::CopyDst;
    textureDesc.viewFormatCount = 0;
    textureDesc.viewFormats = nullptr;
    auto texture = device.CreateTexture(&textureDesc);

    ImageCopyTexture destination;
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = {0, 0, 0};
    destination.aspect = TextureAspect::All;

    TextureDataLayout source;
    source.offset = 0;
    source.bytesPerRow = static_cast<uint32_t>(width) * 4;
    source.rowsPerImage = static_cast<uint32_t>(height);

    queue.WriteTexture(&destination, imageData, width*height*4, &source, &textureDesc.size);

    stbi_image_free(imageData);

    return texture;
}
