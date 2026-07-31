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

struct CameraData
{
    proj: mat4x4<f32>,
    view: mat4x4<f32>
}
@group(0) @binding(0) var<uniform> camera: CameraData;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput
{
    var out: VertexOutput;
    out.position = camera.proj * camera.view * vec4f(in.position, 1.0);
    out.color = in.color;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f
{
    return in.color;
}