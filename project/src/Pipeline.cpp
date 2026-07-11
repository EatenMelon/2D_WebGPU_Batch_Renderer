#include "Pipeline.h"
#include "DataTypes.h"

wgpu::Pipeline::Pipeline(const Shader& shader, const BindGroupLayout* bindGroupLayout)
	: m_Shader{ &shader }
	, m_BindGroupLayout{ bindGroupLayout }
{
	if (m_BindGroupLayout != nullptr && (m_Shader->GetGraphicsContext() != m_BindGroupLayout->GetGraphicsContext()))
	{
		throw std::runtime_error("The graphics context of the shader and the bindgroup layout don't match!");
	}

	WGPURenderPipelineDescriptor desc{};
	desc.nextInChain = nullptr;

	// describe vertex pipeline state
	desc.vertex.bufferCount = 0;
	desc.vertex.buffers = nullptr;

	desc.vertex.module = m_Shader->GetShaderModule();
	desc.vertex.entryPoint = WGPUStringView("vs_main", 7);
	desc.vertex.constantCount = 0;
	desc.vertex.constants = nullptr;

	// describe primitive pipeline state
	desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
	desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
	desc.primitive.frontFace = WGPUFrontFace_CCW;
	desc.primitive.cullMode = WGPUCullMode_Back;

	// describe fragment pipeline state
	WGPUFragmentState fragmentState{};
	fragmentState.module = m_Shader->GetShaderModule();
	fragmentState.entryPoint = WGPUStringView("fs_main", 7);
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;
	
	// blending
	WGPUBlendState blendState{};
	blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
	blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
	blendState.color.operation = WGPUBlendOperation_Add;

	blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
	blendState.alpha.dstFactor = WGPUBlendFactor_One;
	blendState.alpha.operation = WGPUBlendOperation_Add;

	WGPUColorTargetState colorTarget{};
	colorTarget.format = m_Shader->GetGraphicsContext()->GetSurfaceFormat();
	colorTarget.blend = &blendState;
	colorTarget.writeMask = WGPUColorWriteMask_All;

	// We have only one target because our render pass has only one output color
	// attachment.
	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;

	desc.fragment = &fragmentState;

	// describe stencil/depth pipeline stage
	desc.depthStencil = nullptr;

	// describe multi sampling state
	desc.multisample.count = 1;
	desc.multisample.mask = ~0u;
	desc.multisample.alphaToCoverageEnabled = false;

	auto device = m_Shader->GetGraphicsContext()->GetDevice();
	m_Pipeline = wgpuDeviceCreateRenderPipeline(device, &desc);
}

wgpu::Pipeline::~Pipeline() noexcept
{
	wgpuRenderPipelineRelease(m_Pipeline);
}
