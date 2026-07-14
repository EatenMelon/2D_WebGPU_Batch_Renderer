#include "Pipeline.h"
#include "DataTypes.h"

wgpu::Pipeline::Pipeline(const Shader& shader, const BindGroupLayout* bindGroupLayout)
	: m_Shader{ &shader }
	, m_BindGroupLayout{ bindGroupLayout }
{
	if (m_BindGroupLayout != nullptr)
	{
		if (m_Shader->GetGraphicsContext() != m_BindGroupLayout->GetGraphicsContext())
		{
			throw std::runtime_error("The graphics context of the shader and the bindgroup layout don't match!");
		}
		else if (!m_BindGroupLayout->IsLocked())
		{
			throw std::runtime_error("Pipelines can't use unlocked bind group layouts!");
		}
	}

	WGPURenderPipelineDescriptor desc{};
	desc.nextInChain = nullptr;

	// describe vertex pipeline state
	desc.vertex.bufferCount = 0;
	desc.vertex.buffers = nullptr;

	WGPUVertexBufferLayout vertexBufferLayout{};
	std::vector<WGPUVertexAttribute> vertexAttribs(3);
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

		vertexBufferLayout.arrayStride = sizeof(Vertex);
		vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;
	}
	desc.vertex.bufferCount = 1;
	desc.vertex.buffers = &vertexBufferLayout;

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
