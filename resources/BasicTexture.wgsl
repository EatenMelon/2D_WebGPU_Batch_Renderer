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

@group(0) @binding(1) var texture: texture_2d<f32>;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput
{
    var out: VertexOutput;
    out.position = vec4f(in.position, 1.0);
    out.color = in.color;
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f
{
    let size = textureDimensions(texture);
    let coord = vec2i(in.uv * vec2f(size));
    let color = textureLoad(texture, coord, 0);
    let linearColor = pow(color, vec4f(2.2));

    return linearColor * in.color;
}