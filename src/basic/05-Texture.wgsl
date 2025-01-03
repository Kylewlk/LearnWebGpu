
struct Uniforms {
  mvp : mat4x4f,
}

struct VertexOutput
{
	@builtin(position) position: vec4f,
	@location(0) uv: vec2f,
}

@group(0) @binding(0) var<uniform> mvp: mat4x4f;
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

@vertex fn vs(@location(0) pos : vec4f,
              @location(1) uv : vec2f
            ) -> VertexOutput {
    var output : VertexOutput;
    output.position = mvp * pos;
    output.uv = uv;
    return output;
}

@fragment fn fs(@location(0) uv: vec2f,) -> @location(0) vec4f {
    return textureSample(myTexture, mySampler, uv);
}