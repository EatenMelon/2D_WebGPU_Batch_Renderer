#include "BuiltinResources.h"

#include <Shader.h>
#include "BindGroupLayout.h"
#include <Pipeline.h>
#include <Material.h>
#include <Sampler.h>
#include <DataTypes.h>

const std::string_view wgpu::BuiltinResources::m_SolidColorSource
{
    "struct VertexInput\n"
	"{\n"
	"    @location(0) position: vec3f,\n"
	"    @location(1) color: vec4f,\n"
	"    @location(2) uv: vec2f,\n"
	"};\n"
	"struct VertexOutput\n"
	"{\n"
	"    @builtin(position) position: vec4f,\n"
	"    @location(0) color: vec4f,\n"
	"    @location(1) uv: vec2f,\n"
	"}\n"
	"struct CameraData\n"
	"{\n"
	"    proj: mat4x4<f32>,\n"
	"    view : mat4x4<f32>\n"
	"}\n"
	"@group(0) @binding(0) var<uniform> camera: CameraData;\n"
	"@vertex\n"
	"fn vs_main(in: VertexInput) -> VertexOutput\n"
	"{\n"
	"    var out : VertexOutput;\n"
	"    out.position = camera.proj * camera.view * vec4f(in.position, 1.0);\n"
	"    out.color = in.color;\n"
	"    return out;\n"
	"}\n"
	"@fragment\n"
	"fn fs_main(in: VertexOutput) -> @location(0) vec4f\n"
	"{\n"
	"	 let linearColor = vec4f(pow(in.color.rgb, vec3f(2.2)), in.color.a); \n"
	"    return linearColor;\n"
	"}\n"
};

const std::string_view wgpu::BuiltinResources::m_TextureSource
{
	"struct VertexInput\n"
	"{\n"
	"	@location(0) position: vec3f,\n"
	"	@location(1) color: vec4f,\n"
	"	@location(2) uv: vec2f,\n"
	"};\n"
	"struct VertexOutput\n"
	"{\n"
	"	@builtin(position) position: vec4f,\n"
	"	@location(0) color: vec4f,\n"
	"	@location(1) uv: vec2f,\n"
	"}\n"
	"struct CameraData\n"
	"{\n"
	"	proj: mat4x4<f32>,\n"
	"	view : mat4x4<f32>\n"
	"}\n"
	"@group(0) @binding(0) var<uniform> camera: CameraData;\n"
	"@group(0) @binding(1) var texture : texture_2d<f32>;\n"
	"@group(0) @binding(2) var textureSampler : sampler;\n"
	"@vertex\n"
	"fn vs_main(in: VertexInput) -> VertexOutput\n"
	"{\n"
	"	var out : VertexOutput;\n"
	"	out.position = camera.proj * camera.view * vec4f(in.position, 1.0);\n"
	"	out.color = in.color;\n"
	"	out.uv = in.uv;\n"
	"	return out;\n"
	"}\n"
	"@fragment\n"
	"fn fs_main(in: VertexOutput) -> @location(0) vec4f\n"
	"{\n"
	"	let color = textureSample(texture, textureSampler, in.uv);\n"
	"	if (color.a < 0.9)\n"
	"	{\n"
	"		discard;\n"
	"	}\n"
	"	let linearColor = vec4f(pow(color.rgb, vec3f(2.2)), color.a);\n"
	"	return linearColor * in.color;\n"
	"}\n"
};

wgpu::BuiltinResources::BuiltinResources(const Renderer2D& renderer)
	: m_Renderer{ &renderer }
{
	// resources needed for solid color rendering for shapes
	m_SolidColorShader = std::make_unique<Shader>(*m_Renderer, m_SolidColorSource.data(), Shader::ParsingMethod::FromString);

	m_SolidColorLayout = std::make_unique<BindGroupLayout>();

	m_SolidColorLayout->AddUniformEntry<CameraData>(0, BindingVisibility::VertexShaderStage);
	m_SolidColorLayout->ConfirmLayout(*m_Renderer->GetContext());

	m_SolidColorPipeline = std::make_unique<Pipeline>(*m_SolidColorShader.get(), false, m_SolidColorLayout.get());
	m_SolidColorMaterial = std::make_unique<Material>(*m_SolidColorPipeline.get());

	// resources needed for basic texture rendering
	m_TextureShader = std::make_unique<Shader>(*m_Renderer, m_TextureSource.data(), Shader::ParsingMethod::FromString);

	m_TextureLayout = std::make_unique<BindGroupLayout>();
	m_TextureLayout->AddUniformEntry<CameraData>(0, BindingVisibility::VertexShaderStage);
	m_TextureLayout->AddTextureEntry(1);
	m_TextureLayout->AddSamplerEntry(2);
	m_TextureLayout->ConfirmLayout(*m_Renderer->GetContext());

	m_TexturePipeline = std::make_unique<Pipeline>(*m_TextureShader.get(), true, m_TextureLayout.get());

	// samplers
	m_NearestSampler = std::make_unique<Sampler>(*m_Renderer->GetContext(), Sampler::Preset::Nearest);
	m_LinearSampler = std::make_unique<Sampler>(*m_Renderer->GetContext(), Sampler::Preset::Linear);
}

wgpu::BuiltinResources::~BuiltinResources() noexcept = default;

wgpu::Material* wgpu::BuiltinResources::GetSolidColorMaterial() const
{
	return m_SolidColorMaterial.get();
}

std::unique_ptr<wgpu::Material> wgpu::BuiltinResources::CreateTextureMaterial() const
{
	return std::make_unique<Material>(*m_TexturePipeline.get());
}

wgpu::Sampler* wgpu::BuiltinResources::GetNearestSampler() const
{
	return m_NearestSampler.get();
}

wgpu::Sampler* wgpu::BuiltinResources::GetLinearSampler() const
{
	return m_LinearSampler.get();
}
