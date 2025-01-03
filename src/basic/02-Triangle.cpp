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

    TriangleApplication() : Application(false, __FILE__)
    {
        this->setup();
    }
    ~TriangleApplication() override = default;

    void clean() override
    {
        this->pipeline = nullptr;

        Application::clean();
    }

    bool setup()
    {
        using namespace wgpu;
        constexpr const char* shaderCode = R"(
            struct VertexOutput {
              @builtin(position) Position : vec4f,
              @location(0) fragColor: vec4f,
            }

            @vertex
            fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> VertexOutput {
                var output : VertexOutput;
                if (in_vertex_index == 0u) {
                    output.Position = vec4f(-0.5, -0.5, 0.0, 1.0);
                    output.fragColor = vec4f(1.0, 0.0, 0.0, 1.0);
                } else if (in_vertex_index == 1u) {
                    output.Position = vec4f(0.5, -0.5, 0.0, 1.0);
                    output.fragColor = vec4f(0.0, 1.0, 0.0, 1.0);
                } else {
                    output.Position = vec4f(0.0, 0.5, 0.0, 1.0);
                    output.fragColor = vec4f(0.0, 0.0, 1.0, 1.0);
                }
                return output;
            }

            @fragment
            fn fs_main( @location(0) fragColor: vec4f ) -> @location(0) vec4f {
                return fragColor;
            }
        )";
        ShaderModuleWGSLDescriptor shaderCodeDescriptor{};
        shaderCodeDescriptor.code = shaderCode;
        ShaderModuleDescriptor shaderDescriptor{};
        shaderDescriptor.nextInChain = &shaderCodeDescriptor;
        auto shaderModule = this->device.CreateShaderModule(&shaderDescriptor);

        RenderPipelineDescriptor descriptor{};
        descriptor.vertex.bufferCount = 0;
        descriptor.vertex.buffers = nullptr;

        descriptor.vertex.module = shaderModule;
        descriptor.vertex.entryPoint = "vs_main";
        descriptor.vertex.constantCount = 0;
        descriptor.vertex.constants = nullptr;

        descriptor.primitive.topology = PrimitiveTopology::TriangleList;
        descriptor.primitive.stripIndexFormat = IndexFormat::Undefined;
        descriptor.primitive.frontFace = FrontFace::CW;
        descriptor.primitive.cullMode = CullMode::None;

        FragmentState fragmentState;
        fragmentState.module = shaderModule;
        fragmentState.entryPoint = "fs_main";
        fragmentState.constantCount = 0;
        fragmentState.constants = nullptr;

        ColorTargetState colorTarget;
        colorTarget.format = surfaceFormat;
        colorTarget.blend = nullptr;
        colorTarget.writeMask = ColorWriteMask::All; // We could write to only some of the color channels.

        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;
        descriptor.fragment = &fragmentState;

        descriptor.depthStencil = nullptr;

        descriptor.multisample.count = 1;
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
        colorAttachment.view = this->getNextSurfaceTextureView();
        colorAttachment.loadOp = LoadOp::Clear;
        colorAttachment.storeOp = StoreOp::Store;
        colorAttachment.clearValue = {0.2, 0.2, 0.2, 1.0};

        RenderPassDescriptor renderPassDescriptor{};
        renderPassDescriptor.colorAttachments = &colorAttachment;
        renderPassDescriptor.colorAttachmentCount = 1;

        auto encoder = device.CreateCommandEncoder();

        auto pass = encoder.BeginRenderPass(&renderPassDescriptor);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();
        auto commands =  encoder.Finish();
        this->queue.Submit(1, &commands);

        this->surface.Present();

        this->setTitleFps();
    }

    wgpu::RenderPipeline pipeline{};
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