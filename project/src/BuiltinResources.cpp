#include "BuiltinResources.h"

#include "Shader.h"
#include "BindGroupLayout.h"
#include "Pipeline.h"
#include "Material.h"

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
	"    return in.color;\n"
	"}\n"
};

wgpu::BuiltinResources::BuiltinResources(const Renderer2D& renderer)
	: m_Renderer{ &renderer }
{
	// resources need for solid color rendering for shapes
	m_SolidColorShader = std::make_unique<Shader>(*m_Renderer, m_SolidColorSource.data(), Shader::ParsingMethod::FromString);

	m_SolidColorLayout = std::make_unique<BindGroupLayout>();
	m_SolidColorLayout->AddUniformEntry<CameraData>(0, BindingVisibility::VertexShaderStage);
	m_SolidColorLayout->ConfirmLayout(*m_Renderer->GetContext());

	m_SolidColorPipeline = std::make_unique<Pipeline>(*m_SolidColorShader.get(), m_SolidColorLayout.get());
	m_SolidColorMaterial = std::make_unique<Material>(*m_SolidColorPipeline.get());
}

wgpu::BuiltinResources::~BuiltinResources() noexcept = default;

wgpu::Material* wgpu::BuiltinResources::GetMaterial(Type resourceType) const
{
	switch (resourceType)
	{
	case wgpu::BuiltinResources::Type::SolidColor:
		return m_SolidColorMaterial.get();

	case wgpu::BuiltinResources::Type::Texture:
		break;

	default:
		break;
	}

	return nullptr;
}
