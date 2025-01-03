//
// Created by DELL on 2024/12/11.
//

#include <format>
#include <common/Application.h>

class TriangleApplication : public Application
{
public:

    TriangleApplication() : Application(false, __FILE__)
    {
        this->setup();
    }
    ~TriangleApplication() override = default;

    void clean() override
    {
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
            0.0f, 0.8f, 0.0f, 1.0f,
            -0.2f, -0.5f, 0.0f, 1.0f,
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
        // colorTarget.blend = &blendState;
        colorTarget.writeMask = ColorWriteMask::All; // We could write to only some of the color channels.

        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;
        descriptor.fragment = &fragmentState;

        descriptor.depthStencil = nullptr;

        descriptor.multisample.count = 4;
        descriptor.multisample.mask = ~0u;
        descriptor.multisample.alphaToCoverageEnabled = false;

        descriptor.layout = nullptr;

        pipeline = device.CreateRenderPipeline(&descriptor);

        return true;
    }

    void render() override
    {
        glfwPollEvents();

        using namespace wgpu;

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
};


int main()
{
    TriangleApplication app;
    // app.inspectAdapter();
    // app.inspectDevice();

    while (app.isRunning()) {
        app.pollEvent();
        app.render();
    }

    return 0;
}