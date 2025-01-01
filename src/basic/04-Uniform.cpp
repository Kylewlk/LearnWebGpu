//
// Created by DELL on 2024/12/11.
//

#include <format>
#include <iostream>
#include <string>

#include <common/Application.h>

class TriangleApplication : public Application
{
public:
    struct Uniforms
    {
        math::Mat4 mvp{};
        math::Vec4 color{};
    };


    TriangleApplication() : Application(false, __FILE__)
    {
        this->setup();
    }
    ~TriangleApplication() override = default;

    void clean() override
    {
        this->bindGroup = nullptr;
        this->uniformBuffer = nullptr;
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

        static const float vertexData[12] = {
            0.0f, 0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.0f, 1.0f,
        };
        BufferDescriptor bufferDescriptor{};
        bufferDescriptor.usage = BufferUsage::CopyDst | BufferUsage::Vertex;
        bufferDescriptor.size = sizeof(vertexData);
        vertexBuffer = this->device.CreateBuffer(&bufferDescriptor);
        queue.WriteBuffer(vertexBuffer, 0, vertexData, sizeof(vertexData));

        bufferDescriptor.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
        bufferDescriptor.size = sizeof(Uniforms);
        this->uniformBuffer = this->device.CreateBuffer(&bufferDescriptor);

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

        BindGroupLayoutEntry bindEntry;
        bindEntry.binding = 0;
        bindEntry.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
        bindEntry.buffer.type = BufferBindingType::Uniform;
        bindEntry.buffer.minBindingSize = sizeof(Uniforms);

        BindGroupLayoutDescriptor bgLayoutDescriptor{};
        bgLayoutDescriptor.entryCount = 1;
        bgLayoutDescriptor.entries = &bindEntry;

        BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&bgLayoutDescriptor);

        PipelineLayoutDescriptor pipelineLayoutDescriptor{};
        pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
        pipelineLayoutDescriptor.bindGroupLayouts = &bindGroupLayout;

        descriptor.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);;

        pipeline = device.CreateRenderPipeline(&descriptor);


        // Binding Group
        BindGroupEntry bgEntry{};
        bgEntry.binding = 0;
        bgEntry.buffer = this->uniformBuffer;
        bgEntry.offset = 0;
        bgEntry.size = sizeof(Uniforms);

        BindGroupDescriptor bgDescriptor{};
        bgDescriptor.entryCount = 1;
        bgDescriptor.entries = &bgEntry;
        bgDescriptor.layout = bindGroupLayout;
        this->bindGroup = device.CreateBindGroup(&bgDescriptor);

        return true;
    }

    void render() override
    {
        glfwPollEvents();

        using namespace wgpu;

        // update Uniform;
        Uniforms uniforms;
        auto rotation = static_cast<float>(glfwGetTime() / 10) * math::pi_2;
        rotation = std::fmod(rotation, math::pi_2);
        math::Mat4 aspect = math::scale({float(height)/float(width), 1, 1});
        uniforms.mvp = aspect * math::translate({-0.5f, 0.0f, 0.0f})
                                * math::rotateZ(rotation)
                                * math::scale({0.5f, 0.5f, 1.0f});
        uniforms.color = math::Vec4{ 0.0f, 1.0f, 1.0f, 1.0f };
        this->queue.WriteBuffer(uniformBuffer, 0, &uniforms, sizeof(uniforms));

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
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3);
        pass.End();
        auto commands =  encoder.Finish();
        this->queue.Submit(1, &commands);

        this->surface.Present();

        this->setTitleFps();
    }

    wgpu::Buffer vertexBuffer{};
    wgpu::RenderPipeline pipeline{};
    wgpu::Texture msaaTexture{};
    int sampleCount = 4;

    wgpu::BindGroup bindGroup;
    wgpu::Buffer uniformBuffer;
};


int main()
{
    TriangleApplication app;
    // app.inspectAdapter();
    // app.inspectDevice();

    while (app.IsRunning()) {
        app.pollEvent();
        app.render();
    }

    return 0;
}