//
// Created by DELL on 2024/12/11.
//

#include <format>
#include <filesystem>
#include <iostream>
#include <string>

#include <common/Application.h>

class TriangleApplication : public Application
{
public:

    TriangleApplication() : Application(false, __FILE__)
    {
        this->initUniforms();
        this->setup();
    }
    ~TriangleApplication() override = default;

    void clean() override
    {
        this->bindGroupLayout = nullptr;
        this->bindGroup = nullptr;
        this->uniformBuffer = nullptr;
        this->sampler = nullptr;
        this->imageTexture = nullptr;
        this->textureView = nullptr;
        this->msaaTexture = nullptr;
        this->vertexBuffer = nullptr;
        this->pipeline = nullptr;

        Application::clean();
    }

    bool setup()
    {
        using namespace wgpu;

        TextureDescriptor texDescriptor;
        texDescriptor.size = { static_cast<uint32_t>(this->width), static_cast<uint32_t>(this->height)};
        texDescriptor.format = surfaceFormat;
        texDescriptor.sampleCount = sampleCount;
        texDescriptor.usage = TextureUsage::RenderAttachment;
        texDescriptor.dimension = TextureDimension::e2D;
        this->msaaTexture = device.CreateTexture(&texDescriptor);

        constexpr float vertexData[] = {
            -0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, 1.0f, 0.0f,
        };
        BufferDescriptor bufferDescriptor{};
        bufferDescriptor.usage = BufferUsage::CopyDst | BufferUsage::Vertex;
        bufferDescriptor.size = sizeof(vertexData);
        vertexBuffer = this->device.CreateBuffer(&bufferDescriptor);
        queue.WriteBuffer(vertexBuffer, 0, vertexData, sizeof(vertexData));

        constexpr uint32_t indexData[] = {
            0, 1, 2,
            2, 3, 0,
        };
        bufferDescriptor.usage = BufferUsage::CopyDst | BufferUsage::Index;
        bufferDescriptor.size = sizeof(indexData);
        indexBuffer = this->device.CreateBuffer(&bufferDescriptor);
        queue.WriteBuffer(indexBuffer, 0, indexData, sizeof(indexData));

        std::array<VertexAttribute, 2> vertexAttributes{};
        auto& positionAttrib = vertexAttributes[0];
        positionAttrib.shaderLocation = 0;
        positionAttrib.format = VertexFormat::Float32x2;
        positionAttrib.offset = 0;
        auto& uvAttrib = vertexAttributes[1];
        uvAttrib.shaderLocation = 1;
        uvAttrib.format = VertexFormat::Float32x2;
        uvAttrib.offset = sizeof(float) * 2;
        VertexBufferLayout vertexBufferLayout{};
        vertexBufferLayout.attributeCount = vertexAttributes.size();
        vertexBufferLayout.attributes = vertexAttributes.data();
        vertexBufferLayout.stepMode = VertexStepMode::Vertex;
        vertexBufferLayout.arrayStride = sizeof(float) * 4;

        RenderPipelineDescriptor descriptor{};
        descriptor.vertex.bufferCount = 1;
        descriptor.vertex.buffers = &vertexBufferLayout;

        descriptor.vertex.module = this->shaderModule;
        descriptor.vertex.entryPoint = "vs";
        descriptor.vertex.constantCount = 0;
        descriptor.vertex.constants = nullptr;

        descriptor.primitive.topology = PrimitiveTopology::TriangleList;
        descriptor.primitive.stripIndexFormat = IndexFormat::Undefined;
        descriptor.primitive.frontFace = FrontFace::CW;
        descriptor.primitive.cullMode = CullMode::None;

        FragmentState fragmentState;
        fragmentState.module = this->shaderModule;
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
        // colorTarget.blend = &blendState;
        colorTarget.writeMask = ColorWriteMask::All; // We could write to only some of the color channels.

        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;
        descriptor.fragment = &fragmentState;

        descriptor.depthStencil = nullptr;

        descriptor.multisample.count = 4;
        descriptor.multisample.mask = ~0u;
        descriptor.multisample.alphaToCoverageEnabled = false;


        PipelineLayoutDescriptor pipelineLayoutDescriptor{};
        pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
        pipelineLayoutDescriptor.bindGroupLayouts = &bindGroupLayout;

        descriptor.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);;

        pipeline = device.CreateRenderPipeline(&descriptor);

        return true;
    }

    void initUniforms()
    {
        using namespace wgpu;

        BufferDescriptor bufferDescriptor{};
        bufferDescriptor.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
        bufferDescriptor.size =  sizeof(math::Mat4);
        this->uniformBuffer = this->device.CreateBuffer(&bufferDescriptor);

        this->imageTexture = loadTexture(this->device, this->queue, "assets/1.png");
        TextureViewDescriptor textureViewDescriptor{};
        textureViewDescriptor.aspect = TextureAspect::All;
        textureViewDescriptor.baseArrayLayer = 0;
        textureViewDescriptor.arrayLayerCount = 1;
        textureViewDescriptor.baseMipLevel = 0;
        textureViewDescriptor.mipLevelCount = 1;
        textureViewDescriptor.dimension = TextureViewDimension::e2D;
        textureViewDescriptor.format = this->imageTexture.GetFormat();
        textureView = this->imageTexture.CreateView(&textureViewDescriptor);

        SamplerDescriptor samplerDescriptor{};
        samplerDescriptor.addressModeU = AddressMode::Repeat;
        samplerDescriptor.addressModeV = AddressMode::Repeat;
        samplerDescriptor.addressModeW = AddressMode::Repeat;
        samplerDescriptor.magFilter = FilterMode::Linear;
        samplerDescriptor.minFilter = FilterMode::Linear;
        samplerDescriptor.mipmapFilter = MipmapFilterMode::Linear;
        samplerDescriptor.lodMinClamp = 0.0f;
        samplerDescriptor.lodMaxClamp = 1.0f;
        samplerDescriptor.compare = CompareFunction::Undefined;
        samplerDescriptor.maxAnisotropy = 1;
        this->sampler = device.CreateSampler(&samplerDescriptor);

        // group layout
        std::array<BindGroupLayoutEntry, 3> bindLayoutEntries{};
        auto& uboLayoutEntry = bindLayoutEntries[0];
        uboLayoutEntry.binding = 0;
        uboLayoutEntry.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
        uboLayoutEntry.buffer.type = BufferBindingType::Uniform;
        uboLayoutEntry.buffer.minBindingSize = sizeof(math::Mat4);
        uboLayoutEntry.buffer.hasDynamicOffset = false;

        auto& samplerLayoutEntry = bindLayoutEntries[1];
        samplerLayoutEntry.binding = 1;
        samplerLayoutEntry.visibility = ShaderStage::Fragment;
        samplerLayoutEntry.sampler.type = SamplerBindingType::Filtering;

        auto& imageLayoutEntry = bindLayoutEntries[2];
        imageLayoutEntry.binding = 2;
        imageLayoutEntry.visibility = ShaderStage::Fragment;
        imageLayoutEntry.texture.sampleType = TextureSampleType::Float;
        imageLayoutEntry.texture.viewDimension = TextureViewDimension::e2D;

        BindGroupLayoutDescriptor bgLayoutDescriptor{};
        bgLayoutDescriptor.entryCount = bindLayoutEntries.size();
        bgLayoutDescriptor.entries = bindLayoutEntries.data();

        this->bindGroupLayout = device.CreateBindGroupLayout(&bgLayoutDescriptor);

        // Binding Group
        std::array<BindGroupEntry, 3> bindGroupEntries{};
        auto& uboEntry = bindGroupEntries[0];
        uboEntry.binding = 0;
        uboEntry.buffer = this->uniformBuffer;
        uboEntry.offset = 0;
        uboEntry.size = sizeof(math::Mat4);

        auto& samplerEntry = bindGroupEntries[1];
        samplerEntry.binding = 1;
        samplerEntry.sampler = this->sampler;

        auto& imageEntry = bindGroupEntries[2];
        imageEntry.binding = 2;
        imageEntry.textureView=textureView;

        BindGroupDescriptor bgDescriptor{};
        bgDescriptor.entryCount = bindGroupEntries.size();
        bgDescriptor.entries = bindGroupEntries.data();
        bgDescriptor.layout = bindGroupLayout;
        this->bindGroup = device.CreateBindGroup(&bgDescriptor);
    }

    void render() override
    {
        glfwPollEvents();

        using namespace wgpu;

        // update Uniform;
        math::Mat4 aspect = math::scale({float(height)/float(width), 1, 1});
        float imageWidth = float(imageTexture.GetWidth());
        float imageHeight = float(imageTexture.GetHeight());
        math::Mat4 mvp = aspect * math::scale({float(imageWidth)/float(imageHeight), 1, 1});
        this->queue.WriteBuffer(uniformBuffer, 0, &mvp, sizeof(mvp));

        RenderPassColorAttachment colorAttachment{};
        colorAttachment.view = this->msaaTexture.CreateView();
        colorAttachment.resolveTarget = this->getNextSurfaceTextureView();
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
        pass.SetIndexBuffer(indexBuffer, IndexFormat::Uint32);
        pass.SetBindGroup(0, bindGroup);
        pass.DrawIndexed(6);

        pass.End();
        auto commands =  encoder.Finish();
        this->queue.Submit(1, &commands);

        this->surface.Present();

        this->setTitleFps();
    }

    wgpu::Buffer vertexBuffer{};
    wgpu::Buffer indexBuffer{};
    wgpu::RenderPipeline pipeline{};
    wgpu::Texture msaaTexture{};
    int sampleCount = 4;

    wgpu::BindGroupLayout bindGroupLayout;
    wgpu::BindGroup bindGroup;
    wgpu::Buffer uniformBuffer;
    wgpu::Texture imageTexture;
    wgpu::TextureView textureView;
    wgpu::Sampler sampler;
};


int main()
{
    std::cout << CURRENT_WORKING_DIR << std::endl;
    std::filesystem::current_path(CURRENT_WORKING_DIR);

    TriangleApplication app;
    // app.inspectAdapter();
    // app.inspectDevice();

    while (app.isRunning()) {
        app.pollEvent();
        app.render();
    }

    return 0;
}