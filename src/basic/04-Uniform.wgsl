

@vertex fn vs(@location(0) pos : vec4f) -> @builtin(position) vec4f {
    return pos;
}

@fragment fn fs(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
    return vec4f(1, 0, 0, 1);
}