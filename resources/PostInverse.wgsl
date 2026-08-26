
@group(0) @binding(0) var frameTexture: texture_2d<f32>;

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f
{
    var p = vec2f(0.0, 0.0);
    if (in_vertex_index == 0u)
    {
        p = vec2f(-1.0, 1.0);
    }
    else if (in_vertex_index == 1u)
    {
        p = vec2f(-1.0, -1.0);
    }
    else if (in_vertex_index == 2u)
    {
        p = vec2f(1.0, -1.0);
    }
    else if (in_vertex_index == 3u)
    {
        p = vec2f(1.0, 1.0);
    }
    return vec4f(p, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f
{
    return vec4f(0.0, 0.4, 1.0, 1.0);
}