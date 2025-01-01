
struct Uniforms {
  mvp : mat4x4f,
  color: vec4f
}

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

@vertex fn vs(@location(0) pos : vec4f) -> @builtin(position) vec4f {

    var vPos = uniforms.mvp * pos;

    return vPos;
}

@fragment fn fs(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
    return uniforms.color;
}