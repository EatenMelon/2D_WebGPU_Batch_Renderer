#include <Pipeline.h>
#include <DataTypes.h>

#include <Renderer2D.h>
#include "GraphicsContext.h"

static void SetDefault(WGPUStencilFaceState& stencilFaceState)
{
	stencilFaceState.compare = WGPUCompareFunction_Always;
	stencilFaceState.failOp = WGPUStencilOperation_Keep;
	stencilFaceState.depthFailOp = WGPUStencilOperation_Keep;
	stencilFaceState.passOp = WGPUStencilOperation_Keep;
}

static void SetDefault(WGPUDepthStencilState& depthStencilState)
{
	depthStencilState.format = WGPUTextureFormat_Undefined;
	depthStencilState.depthWriteEnabled = WGPUOptionalBool_False;
	depthStencilState.depthCompare = WGPUCompareFunction_Always;
	depthStencilState.stencilReadMask = 0xFFFFFFFF;
	depthStencilState.stencilWriteMask = 0xFFFFFFFF;
	depthStencilState.depthBias = 0;
	depthStencilState.depthBiasSlopeScale = 0;
	depthStencilState.depthBiasClamp = 0;
	SetDefault(depthStencilState.stencilFront);
	SetDefault(depthStencilState.stencilBack);
}

wgpu::Pipeline::Pipeline(const Shader& shader, Type pipelineType, const BindGroupLayout* bindGroupLayout)
	: m_Shader{ &shader }
	, m_BindGroupLayout{ bindGroupLayout }
	, m_Type{ pipelineType }
{
	auto renderer = shader.GetRenderer();

	if (m_BindGroupLayout != nullptr)
	{
		if (!m_BindGroupLayout->IsLocked())
		{
			throw std::runtime_error("Pipelines can't use unlocked bind group layouts!");
		}
		else if (renderer->GetContext() != m_BindGroupLayout->GetContext())
		{
			throw std::runtime_error("The graphics context of the shader and the bindgroup layout don't match!");
		}

		const bool needsFrameBuffer = m_BindGroupLayout->GetFrameEntryBinding() >= 0;
		if (m_Type == Type::PostProcessing && !needsFrameBuffer)
		{
			throw std::runtime_error("A post-processing shader requires a frame entry!");
		}
	}

	WGPURenderPipelineDescriptor desc{};
	desc.nextInChain = nullptr;

	// describe vertex pipeline state
	desc.vertex.bufferCount = 0;
	desc.vertex.buffers = nullptr;

	WGPUVertexBufferLayout vertexBufferLayout{};
	std::vector<WGPUVertexAttribute> vertexAttribs(3);

	if (m_Type != Type::PostProcessing)
	{
		// position
		vertexAttribs[0].shaderLocation = 0;
		vertexAttribs[0].format = WGPUVertexFormat_Float32x3;
		vertexAttribs[0].offset = 0;

		// color
		vertexAttribs[1].shaderLocation = 1;
		vertexAttribs[1].format = WGPUVertexFormat_Float32x4;
		vertexAttribs[1].offset = sizeof(glm::vec3);

		// uv
		vertexAttribs[2].shaderLocation = 2;
		vertexAttribs[2].format = WGPUVertexFormat_Float32x2;
		vertexAttribs[2].offset = sizeof(glm::vec3) + sizeof(glm::vec4);

		vertexBufferLayout.attributeCount = static_cast<uint32_t>(vertexAttribs.size());
		vertexBufferLayout.attributes = vertexAttribs.data();

		vertexBufferLayout.arrayStride = sizeof(Vertex3D);
		vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;

		desc.vertex.bufferCount = 1;
		desc.vertex.buffers = &vertexBufferLayout;
	}
	else
	{
		desc.vertex.bufferCount = 0;
		desc.vertex.buffers = nullptr;
	}

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
	colorTarget.format = renderer->GetContext()->GetSurfaceFormat();
	colorTarget.blend = &blendState;
	colorTarget.writeMask = WGPUColorWriteMask_All;

	// We have only one target because our render pass has only one output color
	// attachment.
	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;

	desc.fragment = &fragmentState;

	if (m_Type != Type::PostProcessing)
	{
		// describe stencil/depth pipeline stage
		WGPUDepthStencilState depthStencilState{};
		SetDefault(depthStencilState);

		depthStencilState.depthCompare = WGPUCompareFunction_LessEqual;

		if (m_Type == Type::GeometryOpaque)
		{
			depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
		}
		else
		{
			depthStencilState.depthWriteEnabled = WGPUOptionalBool_False;
		}

		WGPUTextureFormat depthTextureFormat = wgpuTextureGetFormat(renderer->GetDepthTexture());
		depthStencilState.format = depthTextureFormat;

		depthStencilState.stencilReadMask = 0;
		depthStencilState.stencilWriteMask = 0;

		desc.depthStencil = &depthStencilState;
	}

	// describe multi sampling state
	desc.multisample.count = 1;
	desc.multisample.mask = ~0u;
	desc.multisample.alphaToCoverageEnabled = false;

	auto device = renderer->GetContext()->GetDevice();

	if (m_BindGroupLayout != nullptr)
	{
		WGPUPipelineLayoutDescriptor layoutDesc{};
		layoutDesc.nextInChain = nullptr;
		layoutDesc.bindGroupLayoutCount = 1;
		const auto layout = m_BindGroupLayout->GetLayout();
		layoutDesc.bindGroupLayouts = &layout;

		desc.layout = wgpuDeviceCreatePipelineLayout(device, &layoutDesc);
	}

	m_Pipeline = wgpuDeviceCreateRenderPipeline(device, &desc);
}

wgpu::Pipeline::~Pipeline() noexcept
{
	wgpuRenderPipelineRelease(m_Pipeline);
}

const wgpu::GraphicsContext* wgpu::Pipeline::GetContext() const
{
	return m_Shader->GetRenderer()->GetContext();
}