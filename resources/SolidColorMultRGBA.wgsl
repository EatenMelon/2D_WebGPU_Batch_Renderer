struct VertexInput
{
    @location(0) position: vec3f,
    @location(1) color: vec4f,
    @location(2) uv: vec2f,
};

struct VertexOutput
{
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
    @location(1) uv: vec2f,
}

struct ColorF
{
    @location(0) r : f32,
    @location(1) g : f32,
    @location(2) b : f32,
    @location(3) a : f32,
}

@group(0) @binding(0) var<uniform> uColorMultiplier : ColorF;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput
{
    var out: VertexOutput;
    out.position = vec4f(in.position, 1.0);
    out.color = in.color;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f
{
    var color : vec4f;
    color.r = uColorMultiplier.r;
    color.g = uColorMultiplier.g;
    color.b = uColorMultiplier.b;
    color.a = uColorMultiplier.a;

    return in.color * color;
}