@group(0) @binding(0)
var frameTexture : texture_2d<f32>;

@vertex
fn vs_main(@builtin(vertex_index) i : u32) -> @builtin(position)
    vec4f
{
    var p = array<vec2f, 3>(
        vec2f(-1.0, -1.0),
        vec2f( 3.0, -1.0),
        vec2f(-1.0,  3.0)
    );

    return vec4f(p[i], 0.0, 1.0);
}

@fragment
fn fs_main(@builtin(position) pos : vec4f) -> @location(0) vec4f
{
    let color = textureLoad(
        frameTexture,
        vec2i(pos.xy),
        0
    );

    let out = vec4f(1 - color.r, 1 - color.g, 1 - color.b, 1);

    return out;
}